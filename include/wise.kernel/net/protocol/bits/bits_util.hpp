#pragma once 

#include <lax/net/detail/buffer/resize_buffer.hpp>
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/flexible.h>
#include <bitsery/flexible/array.h>
#include <bitsery/flexible/deque.h>
#include <bitsery/flexible/list.h>
#include <bitsery/flexible/map.h>
#include <bitsery/flexible/set.h>
#include <bitsery/flexible/string.h>
#include <bitsery/flexible/unordered_map.h>
#include <bitsery/flexible/vector.h>
#include <bitsery/ext/inheritance.h>

namespace bitsery
{
namespace traits
{

template<>
struct ContainerTraits<lax::net::resize_buffer>
	:public StdContainer<lax::net::resize_buffer, true, true>
{
};

template<>
struct BufferAdapterTraits<lax::net::resize_buffer>
	: public StdContainerForBufferAdapter<lax::net::resize_buffer, true>
{
};

} // traits
} // bitsery

namespace lax
{
namespace net
{

using OutputAdapter = bitsery::OutputBufferAdapter<lax::net::resize_buffer>;
using InputAdapter = bitsery::InputBufferAdapter<lax::net::resize_buffer>;

using BitsSerializer = bitsery::BasicSerializer <bitsery::AdapterWriter<OutputAdapter, bitsery::DefaultConfig>>;
using BitsDeserializer = bitsery::BasicDeserializer <bitsery::AdapterReader<InputAdapter, bitsery::DefaultConfig>>;

} // net 
} // lax


