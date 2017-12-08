#ifndef __PACKED_MESSAGE__
#define __PACKED_MESSAGE__
#include "mpack.pb.h"
#include <array>
#include <string>
#include <sstream>
#include <iostream>
namespace MNet{
class MpackMessage{
public:
    enum { header_length = 4 };
    enum { max_body_length = 65500 };
    MpackMessage() : body_length_(0) {}
    const char * data() const{
        return data_;
    }
    
    char * data() {
        return data_;
    }
    const char * body() const {
        return data_ + header_length;
    }

    char * body() {
        return data_ + header_length;
    }
    uint32_t body_length(){
        return body_length_;
    }
    
    uint32_t whole_size(){
        return header_length + body_length_;
    }
    uint32_t decode_header();
    // Encode request in PackedMessage and update body_length at the same time
    bool encode_header(uint32_t l);
    bool encode_mpack(const Mpack& m); 
    Mpack decode_mpack(); 
    
private:
    char  data_[header_length + max_body_length];
    uint32_t body_length_;
};
}
#endif
