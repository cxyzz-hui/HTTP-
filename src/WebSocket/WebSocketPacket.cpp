#include "WebSocketPacket.hpp"
#include "../Buffer.hpp"
#include <arpa/inet.h>

WebSocketPacket::WebSocketPacket()
:fin_(1)        //1表示最后一个分片，代表消息不分包
,rsv1_(0)
,rsv2_(0)
,rsv3_(0)
,opcode_(1)     //默认发送的文本数据
,mask_(0)
,payload_length_(0)
{
    memset(masking_key_ , 0 , sizeof(masking_key_));
}

WebSocketPacket::~WebSocketPacket()
{

}

void WebSocketPacket::reset()
{
    fin_ = 1;
    rsv1_ = 0;
    rsv2_ = 0;
    rsv3_ = 0;
    opcode_ = 1;
    mask_ = 0;
    payload_length_ = 0;
    memset(masking_key_ , 0 , sizeof(masking_key_));
}

//解码帧信息
/*
过程
原始字节流：
[0x81 0x85 0x37 0xFA 0x21 0x3D 0x7F 0x9F 0x4D 0x51 0x58]
      ↓ decodeFrame
还原为应用数据：
"Hello"
*/
void WebSocketPacket::decodeFrame(Buffer* frameBuf , Buffer* output)
{
    const char *msg = frameBuf->peek();

    int pos = 0;
    //获得fin(帧数片)
    fin_ = ((unsigned char)msg[pos] >> 7);          //这个运算过程是固定的
    //获得操作码
    opcode_ = msg[pos] & 0x0f;
    pos++;
    //获取掩码(秘钥)
    mask_ = ((unsigned char)msg[pos] >> 7);
    //获取负载长度(数据大小)
    payload_length_ = msg[pos] & 0x7f;
    pos++;

    if(payload_length_ == 126)
    {
        uint16_t length = 0;
        memcpy(&length , msg + pos , 2);
        pos += 2;
        payload_length_ = ntohs(length);
    }
    else if(payload_length_ == 127)
    {
        uint64_t length = 0;
        memcpy(&length , msg + pos , 8);
        pos += 8;
        payload_length_ = ntohl(length);
    }

    //使用掩码可以防止代理攻击，更具有安全性
    if(mask_ == 1)
    {
        for(int i = 0; i < 4; i++)
        {
            masking_key_[i] = msg[pos + i];
        }

        pos += 4;
    }

    if(mask_ != 1)
    {
        output->append(msg + pos , payload_length_);
    }
    else
    {
        for(uint64_t i = 0; i < payload_length_; i++)
        {
            output->append(msg[pos + i] ^ masking_key_[i % 4], payload_length_);
        }
    }
}

//封装帧
void WebSocketPacket::encodeFrame(Buffer* output , Buffer* data) const
{
    uint8_t onebyte = 0;
    //求传输的数据有多少字节(固定格式)
    onebyte |= (fin_ << 7);
    onebyte |= (rsv1_ << 6);
    onebyte |= (rsv2_ << 5);
    onebyte |= (rsv3_ << 4);
    onebyte |= (opcode_ & 0x0f);
    output->append((char*)&onebyte , 1);

    onebyte = 0;
    //设置掩码
    onebyte |= (mask_ << 7);

    int length = data->readableBytes();

    //这里设计WebSocket底层的一些设计，在WebSocket中少不少于于126是为两中储存模式
    if(length < 126)
    {
        onebyte |= length;
        output->append((char*)&onebyte , 1);
    }
    else if(onebyte == 126)
    {
        onebyte |= length;
        output->append((char*)&onebyte , 1);
    
        auto len = htons(length);
        output->append((char*)&onebyte , 1);
    }
    else if(onebyte == 127)
    {
        onebyte |= length;
        output->append((char*)&onebyte , 1);

        //固定的反编码格式
        onebyte = (payload_length_ >> 56) & 0xFF;
		output->append((char*)&onebyte, 1);
		onebyte = (payload_length_ >> 48) & 0xFF;
		output->append((char*)&onebyte, 1);
		onebyte = (payload_length_ >> 40) & 0xFF;
		output->append((char*)&onebyte, 1);
		onebyte = (payload_length_ >> 32) & 0xFF;
		output->append((char*)&onebyte, 1);
		onebyte = (payload_length_ >> 24) & 0xFF;
		output->append((char*)&onebyte, 1);
		onebyte = (payload_length_ >> 16) & 0xFF;
		output->append((char*)&onebyte, 1);
		onebyte = (payload_length_ >> 8) & 0xFF;
		output->append((char*)&onebyte, 1);
		onebyte = payload_length_ & 0XFF;
		output->append((char*)&onebyte, 1);
    }

    //检查是否有掩码
    if(mask_ == 1)
    {
        output->append((char*)masking_key_ , 4);
        char value = 0;
        for(uint64_t i = 0; i < payload_length_; i++)
        {
            //读取数据，更新位置
            value = *(char*)output->peek();
            data->retrieve(1);

            value ^= masking_key_[i % 4];
            output->append(&value , 1);
        }
    }
    else
    {
        output->append(data->peek(), data->readableBytes());
    }

}

//构建协议头
void WebSocketPacket::addFrameHeader(Buffer* output)
{
    payload_length_ = output->readableBytes() - 14;

    uint8_t onebyte = 0;
    onebyte |= (fin_ << 7);
	onebyte |= (rsv1_ << 6);
	onebyte |= (rsv2_ << 5);
	onebyte |= (rsv3_ << 4);
	onebyte |= (opcode_ & 0x0F);

    //得知此时帧信息是否已经完结
    output->append((char*)&onebyte , 1);

    onebyte = 0;
    onebyte = onebyte | (mask_ << 7);

    int length = payload_length_;

	if (length < 126)
	{
		onebyte |= length;
		output->append((char*)&onebyte, 1);
	}
	else if (length == 126)
	{
		onebyte |= length;
		output->append((char*)&onebyte, 1);

		auto len = htons(length);
		output->append((char*)&len, 2);

	}
	else if (length == 127)
	{
		onebyte |= length;
		output->append((char*)&onebyte, 1);

		// also can use htonll if you have it
		onebyte = (payload_length_ >> 56) & 0xFF;
		output->append((char*)&onebyte, 1);
		onebyte = (payload_length_ >> 48) & 0xFF;
		output->append((char*)&onebyte, 1);
		onebyte = (payload_length_ >> 40) & 0xFF;
		output->append((char*)&onebyte, 1);
		onebyte = (payload_length_ >> 32) & 0xFF;
		output->append((char*)&onebyte, 1);
		onebyte = (payload_length_ >> 24) & 0xFF;
		output->append((char*)&onebyte, 1);
		onebyte = (payload_length_ >> 16) & 0xFF;
		output->append((char*)&onebyte, 1);
		onebyte = (payload_length_ >> 8) & 0xFF;
		output->append((char*)&onebyte, 1);
		onebyte = payload_length_ & 0XFF;
		output->append((char*)&onebyte, 1);
	}
}
