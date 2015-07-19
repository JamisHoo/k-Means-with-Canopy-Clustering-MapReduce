/******************************************************************************
 *  Copyright (c) 2015 Jamis Hoo
 *  Distributed under the MIT license 
 *  (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *  
 *  Project: 
 *  Filename: netflix_movie.h 
 *  Version: 1.0
 *  Author: Jamis Hoo
 *  E-mail: hoojamis@gmail.com
 *  Date: Jul 18, 2015
 *  Time: 22:29:23
 *  Description: 
 *****************************************************************************/
#ifndef NETFLIX_MOVIE_H_
#define NETFLIX_MOVIE_H_

#include <cstdlib>
#include <string>
#include <vector>

class Movie {
public:
    // construct from record string
    // record format: movie_id:user_id1,rating1,user_id2,rating2,...
    // record in single line without any spaces
    // user_ids are already sorted
    Movie(const std::string& record) {
        char* offset;

        movie_id_ = strtoul(record.data(), &offset, 16);

        const char* end = record.data() + record.length();
        while (offset != end) {
            const char* tmp = offset + 1;
            uint32_t user_id = strtoul(tmp, &offset, 16);
            user_ids_.emplace_back(user_id);
            tmp = offset + 1;
            uint8_t rating = strtoul(tmp, &offset, 16);
            ratings_.emplace_back(rating);
        }
    }

    uint32_t movie_id() const { return movie_id_; }
    size_t num_users() const { return user_ids_.size(); }
    uint32_t user_id(size_t i) const { return user_ids_[i]; }
    uint8_t rating(size_t i) const { return ratings_[i]; }
    
    size_t user_match_count(const Movie& mv) const {
        auto first1 = user_ids_.begin();
        auto last1 = user_ids_.end();
        auto first2 = mv.user_ids_.begin();
        auto last2 = mv.user_ids_.end();

        size_t count = 0;
        
        while (first1 != last1 && first2 != last2) {
            if (*first1 < *first2) ++first1;
            else if (*first2 < *first1) ++first2;
            else { ++count; ++first1; ++first2; }
        }

        return count;
    }

private:
    uint32_t movie_id_; 
    std::vector<uint32_t> user_ids_;
    std::vector<uint8_t> ratings_;

};


#endif /* NETFLIX_MOVIE_H_ */
