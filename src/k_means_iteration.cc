/******************************************************************************
 *  Copyright (c) 2015 Jamis Hoo
 *  Distributed under the MIT license 
 *  (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *  
 *  Project: 
 *  Filename: k_means_iteration.cc 
 *  Version: 1.0
 *  Author: Jamis Hoo
 *  E-mail: hjm211324@gmail.com
 *  Date: Jul 19, 2015
 *  Time: 20:55:38
 *  Description: 
 *****************************************************************************/
#include <iostream>
#include <fstream>
#include <cassert>
#include <unordered_map>
#include <algorithm>

#include "hadoop/Pipes.hh"
#include "hadoop/TemplateFactory.hh"
#include "hadoop/StringUtils.hh"

#include "netflix_movie.h"

constexpr size_t canopy_threshold = 2;
const std::string canopy_centers_path = "canopy_centers";
const std::string k_means_center_path = "k_means_centers";

inline std::string to_hex_string(const size_t x) {
    char buff[32] = { 0 };
    sprintf(buff, "%zx", x);
    
    return buff;
}

inline std::vector<uint32_t> string_split(const std::string& str) {
    std::vector<uint32_t> numbers;

    char* offset;

    const char* end = str.data() + str.length();
    const char* tmp = str.data();
    while (tmp < end) {
        uint32_t movie_id = strtoul(tmp, &offset, 16);
        numbers.emplace_back(movie_id);
        tmp = offset + 1;
    }

    return numbers;
}

class kMeansMapper: public HadoopPipes::Mapper {
public:
    kMeansMapper(HadoopPipes::TaskContext& /* context */) {
        load_canopy_centers();
        load_kmeans_centers();

        // no use anymore
        canopy_centers.clear();
    }

    void map(HadoopPipes::MapContext& context) {
        std::string input_value = context.getInputValue(); 

        std::string movie_string = 
            input_value.substr(0, input_value.find_first_of('\t')) + ':' +
            input_value.substr(input_value.find_first_of(';') + 1);

        Movie movie = movie_string;

        // std::cout << "Load input movie: " << movie.to_string() << std::endl;

        size_t start_pos = input_value.find_first_of('\t');
        size_t end_pos = input_value.find_first_of(';', start_pos);
        std::vector<uint32_t> canopy_ids = string_split(input_value.substr(start_pos + 1, end_pos - start_pos - 1));
        
        // std::cout << "canopy_ids: "; for (const auto i: canopy_ids) std::cout << i << ' '; std::cout << std::endl;

        float max_distance = -1;
        const Movie* max_distance_movie = nullptr;
        for (const auto canopy_id: canopy_ids) {
            if (canopy_id == movie.movie_id()) continue;
            for (const auto k_means_center: centers[canopy_id]) {
                float distance = movie.cos_distance(k_means_centers[k_means_center]);
                // std::cout << "Distance with " << k_means_center.to_string() << " is " << distance;
                if (distance > max_distance) {
                    // std::cout << " is max. ";
                    max_distance = distance;
                    max_distance_movie = &k_means_centers[k_means_center];
                }
                // std::cout << std::endl;
            }
        }

        if (max_distance_movie == nullptr) return;

        std::string emit_key = max_distance_movie->to_string();
        std::string emit_value = movie_string;

        // std::cout << "emit_key = " << emit_key << std::endl;
        // std::cout << "emit_value = " << emit_value << std::endl;

        context.emit(emit_key, emit_value);
    }

private:   
    void load_canopy_centers() {
        std::ifstream fin(canopy_centers_path);

        std::string line;
        while (std::getline(fin, line)) {
            canopy_centers.emplace_back(line); 
            // std::cout << "Load cannopy center: " << canopy_centers.back().to_string() << std::endl;
        }

    }

    void load_kmeans_centers() {
        std::ifstream fin(k_means_center_path);

        std::string line;
        while (std::getline(fin, line)) {
            k_means_centers.push_back(line);
            Movie& k_means_center = k_means_centers.back();

            // std::cout << "k-means center: " << k_means_center.to_string() << " ";
            for (const auto& canopy_center: canopy_centers) 
                if (k_means_center.user_match_count(canopy_center) > canopy_threshold) {
                    // std::cout << "applied to canopy " << canopy_center.to_string() << " ";
                    auto ite = centers.emplace(canopy_center.movie_id(), std::vector<uint32_t>()).first;
                    ite->second.push_back(k_means_centers.size() - 1);
                }
            // std::cout << std::endl;
        }
    }
    
    std::vector<Movie> k_means_centers;
    std::vector<Movie> canopy_centers;
    // canopy movie_id, vector of k_means_centers index
    std::unordered_map< uint32_t, std::vector<uint32_t> > centers;
};

class kMeansReducer: public HadoopPipes::Reducer {
public:
    kMeansReducer(const HadoopPipes::TaskContext& /* context */) { }

    void reduce(HadoopPipes::ReduceContext& context) {
        Movie k_means_center(context.getInputKey());
#ifndef LAST_ITERATION
        // <user id, < number of users, total user ratings > >
        std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t> > new_features;

        while (context.nextValue()) {
            Movie movie(context.getInputValue());

            for (size_t i = 0; i < movie.num_users(); ++i) {
                auto ite = new_features.emplace(movie.user_id(i), std::make_pair(0, 0)).first;

                ite->second.first += 1;
                ite->second.second += movie.rating(i);
            }
        }

        std::vector< std::pair<uint32_t, uint32_t> > new_features_vec;
        for (const auto i: new_features)
            new_features_vec.push_back({ i.first, i.second.second / i.second.first });

        sort(new_features_vec.begin(), new_features_vec.end());

        std::string emit_key = to_hex_string(k_means_center.movie_id());

        std::string emit_value;
        for (const auto i: new_features_vec)
            emit_value += to_hex_string(i.first) + ',' + to_hex_string(i.second) + ',';

        if (emit_value.size()) {
            emit_value.pop_back();
            context.emit(emit_key, emit_value);
        }
#else
        std::string emit_value;
        while (context.nextValue()) {
            Movie movie(context.getInputValue());
            emit_value += to_hex_string(movie.movie_id()) + ',';
        }
        if (emit_value.length()) emit_value.pop_back();
        context.emit(emit_value, "");
#endif
    }
};

int main(int, char**) {
    return HadoopPipes::runTask(HadoopPipes::TemplateFactory<kMeansMapper, kMeansReducer>());
}


