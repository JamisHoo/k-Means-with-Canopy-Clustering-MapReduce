/******************************************************************************
 *  Copyright (c) 2015 Jamis Hoo
 *  Distributed under the MIT license 
 *  (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *  
 *  Project: 
 *  Filename: label_data.cc 
 *  Version: 1.0
 *  Author: Jamis Hoo
 *  E-mail: hjm211324@gmail.com
 *  Date: Jul 19, 2015
 *  Time: 17:58:55
 *  Description: 
 *****************************************************************************/
#include <iostream>
#include <fstream>
#include <cassert>

#include "hadoop/Pipes.hh"
#include "hadoop/TemplateFactory.hh"
#include "hadoop/StringUtils.hh"

#include "netflix_movie.h"

constexpr size_t canopy_threshold = 2;
const std::string canopy_centers_path = "canopy_output";

inline std::string to_hex_string(const size_t x) {
    char buff[32] = { 0 };
    sprintf(buff, "%zx", x);
    
    return buff;
}

class LabelDataMapper: public HadoopPipes::Mapper {
public:
    LabelDataMapper(HadoopPipes::TaskContext& /* context */) {
        load_canopy_centers();
    }

    // emit key: movie id 
    // emit value: canopy id1, canopy id2, canopy id3 ...; user_id1, rating1, user_id2, rating2, ...
    // without any spaces
    void map(HadoopPipes::MapContext& context) {
        Movie movie = context.getInputValue();         

        std::string emit_value;

        for (const auto& mv: canopy_centers) 
            if (movie.user_match_count(mv) > canopy_threshold)
                emit_value += to_hex_string(mv.movie_id()) + ',';

        if (emit_value.length() == 0) return;

        emit_value.back() = ';';

        size_t pos = context.getInputValue().find_first_of(':');

        std::string emit_key = context.getInputValue().substr(0, pos);
        emit_value += context.getInputValue().substr(pos + 1);

        context.emit(emit_key, emit_value);
    }

private: 
    void load_canopy_centers() {
        std::ifstream fin(canopy_centers_path);

        std::string line;
        while (std::getline(fin, line)) 
            canopy_centers.emplace_back(line);

    }

    std::vector<Movie> canopy_centers;
};

class DoNothingReducer: public HadoopPipes::Reducer {
public:
    DoNothingReducer(const HadoopPipes::TaskContext& /* context */) { }

    void reduce(HadoopPipes::ReduceContext& context) {
        bool once_flag = false;

        while (context.nextValue()) {
            assert(once_flag == false);
            once_flag = true;
            context.emit(context.getInputKey(), context.getInputValue());
        }
    }
};



int main(int, char**) {

    return HadoopPipes::runTask(HadoopPipes::TemplateFactory<LabelDataMapper, DoNothingReducer>());
}

