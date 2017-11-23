#ifndef PACKED_MESSAGE_HPP
#define PACKED_MESSAGE_HPP
#include <prototmp/request.pb.h>
#include <array>
#include <string>
#include <sstream>
#include <iostream>

class PackedMessage{
public:
    enum { header_length = 4 };
    enum { max_body_length = 512 };
    typedef std::array<char, header_length+max_body_length> DataArray;

    PackedMessage() : body_length_(0) {}
    const DataArray & data() const{
        return data_;
    }
    
    DataArray & data() {
        return data_;
    }

    size_t body_length(){
        return body_length_;
    }
    
    size_t whole_size(){
        return header_length + body_length_;
    }
    size_t decode_header();
    Request decode_request(); 
    // Encode request in PackedMessage and update body_length at the same time
    void encode_request(const Request & r);



    
private:
    DataArray data_;
    std::size_t body_length_;
};

#endif
