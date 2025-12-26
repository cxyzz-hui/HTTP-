#pragma
#ifndef WEBSOCKETPACKET_HPP
#define WEBSOCKETPACKET_HPP

//数据帧的解析

#include <stdint.h>
#include <string.h>

class Buffer;

enum WSOpcodeType : uint8_t
{
    WSOpcode_Continue = 0x0,
    WSOpcode_Text = 0x1,
    WSOpcode_Binary = 0x2,
    WSOpcode_Close = 0x8,
    WSOpcode_Ping = 0x9,
    WSOpcode_Pong = 0xA
};

class WebSocketPacket
{
public:
    WebSocketPacket();
    ~WebSocketPacket();

    void reset();

    void decodeFrame(Buffer* frameBuffer , Buffer* output);
    void encodeFrame(Buffer* output , Buffer* data) const;

    void addFrameHeader(Buffer* output);

public:
    //工具函数
    void Set_fin(uint8_t fin) { fin_ = fin; }
    void Set_rsv1(uint8_t rsv1) { rsv1_ = rsv1; }
    void Set_rsv2(uint8_t rsv2) { rsv2_ = rsv2; }
    void Set_rsv3(uint8_t rsv3) { rsv3_ = rsv3; }
    void Set_opcode_(uint8_t opcode) { opcode_ = opcode; }
    void Set_mask(uint8_t mask) { mask_ = mask_; }
    void Set_payload_length(uint64_t lenth) { payload_length_ = lenth; }

    uint8_t Get_fin() const { return fin_; }
    uint8_t Get_rsv1() const { return rsv1_; }
    uint8_t Get_rsv2() const { return rsv2_; }
    uint8_t Get_rsv3() const { return rsv3_; }
    uint8_t Get_opcode() const { return opcode_; }
    uint8_t Get_maks() const { return mask_; }
    uint64_t Get_payload_length() const { return payload_length_; }

private:

    uint8_t fin_;          // 1位，表示是否是消息的最后一个分片(如果内容过长是会分帧发送)
    uint8_t rsv1_;         // 1位，扩展用，一般为0
    uint8_t rsv2_;         // 1位，扩展用，一般为0
    uint8_t rsv3_;         // 1位，扩展用，一般为0
    uint8_t opcode_;       // 4位，操作码，定义帧类型(发送什么类型的内容)
    uint8_t mask_;         // 1位，是否使用掩码
    uint8_t masking_key_[4]; // 4字节，掩码密钥
    uint64_t payload_length_; // 负载数据长度(消息有多长)

};


#endif