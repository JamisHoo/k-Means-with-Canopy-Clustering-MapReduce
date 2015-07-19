/******************************************************************************
 *  Copyright (c) 2015 Jamis Hoo
 *  Distributed under the MIT license 
 *  (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *  
 *  Project: 
 *  Filename: canopy_clustering.cc 
 *  Version: 1.0
 *  Author: Jamis Hoo
 *  E-mail: hoojamis@gmail.com
 *  Date: Jul 18, 2015
 *  Time: 22:23:28
 *  Description: 
 *****************************************************************************/
#include <iostream>
#include <cassert>

#include "hadoop/Pipes.hh"
#include "hadoop/TemplateFactory.hh"
#include "hadoop/StringUtils.hh"

#include "netflix_movie.h"

constexpr size_t canopy_threshold = 10;

class CanopyMapper: public HadoopPipes::Mapper {
public:
    CanopyMapper(HadoopPipes::TaskContext& /* context */) { }

    void map(HadoopPipes::MapContext& context) {
        Movie movie(context.getInputValue());
        bool cover_by_other_canopies = false;

        for (const auto& mv: canopy_centers) 
            if (movie.user_match_count(mv) > canopy_threshold) {
                cover_by_other_canopies = true;
                break;
            }

        if (!cover_by_other_canopies) {
            size_t colon_pos = context.getInputValue().find_first_of(':');
            std::string emit_key = context.getInputValue().substr(0, colon_pos);
            std::string emit_value = context.getInputValue().substr(colon_pos + 1);
            context.emit(emit_key, emit_value);

            /*
            std::cout << "Found center: " << std::hex 
                      << movie.movie_id() << "(" << movie.num_users() << ")" 
                      << std::endl;
            */
            canopy_centers.emplace_back(movie); 
        }
    }

private:
    std::vector<Movie> canopy_centers;
};

class CanopyReducer: public HadoopPipes::Reducer {
public:
    CanopyReducer(HadoopPipes::TaskContext& /* context */) { }

    void reduce(HadoopPipes::ReduceContext& context) {
        bool once_flag = false;
        while (context.nextValue()) {
            assert(once_flag == false);
            once_flag = true;

            Movie movie(context.getInputKey() + ":" + context.getInputValue());

            /*
            if (movie.movie_id() == 1) {
                std::cout << movie.movie_id() << ":";
                for (size_t i = 0; i < movie.num_users(); ++i)
                    std::cout << movie.user_id(i) << "," << movie.rating(i) << ",";
                std::cout << std::endl;
            }
            */

            bool cover_by_other_canopies = false;

            for (const auto& mv: canopy_centers)
                if (movie.user_match_count(mv) > canopy_threshold) {
                    cover_by_other_canopies = true;
                    /*
                    std::cout << "Merge center " << std::hex
                              << movie.movie_id() << "(" << movie.num_users() << ")" 
                              << " to " << mv.movie_id() << "(" << mv.num_users() << ")" << std::endl;
                    */
                    break;
                }
            
            if (!cover_by_other_canopies) {
                context.emit(context.getInputKey(), context.getInputValue());

                canopy_centers.emplace_back(movie);
            }
        }
    }

private:
    std::vector<Movie> canopy_centers;
};


int main() {
    return HadoopPipes::runTask(HadoopPipes::TemplateFactory<CanopyMapper, CanopyReducer>());
}
