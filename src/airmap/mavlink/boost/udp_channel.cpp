// AirMap Platform SDK
// Copyright © 2018 AirMap, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//   http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <airmap/mavlink/boost/udp_channel.h>

namespace {
constexpr const char* component{"airmap::mavlink::UdpChannel"};
}  // namespace

airmap::mavlink::boost::UdpChannel::UdpChannel(const std::shared_ptr<Logger>& logger,
                                               const std::shared_ptr<::boost::asio::io_service>& io_service,
                                               std::uint16_t port)
    : log_{logger},
      io_service_{io_service},
      socket_{*io_service_, ::boost::asio::ip::udp::endpoint{::boost::asio::ip::udp::v4(), port}} {
}

void airmap::mavlink::boost::UdpChannel::start_impl() {
  auto sp      = shared_from_this();
  using std::placeholders::_1;
  using std::placeholders::_2;

  socket_.async_receive_from(::boost::asio::buffer(buffer_), endpoint_,
                             std::bind(&UdpChannel::handle_read, shared_from_this(), _1, _2));

  state_ = 1;
}

void airmap::mavlink::boost::UdpChannel::stop_impl() {
  socket_.cancel();
}

std::size_t airmap::mavlink::boost::UdpChannel::EncodedBuffer::size() const {
  return size_;
}

void airmap::mavlink::boost::UdpChannel::EncodedBuffer::set_size(std::size_t size) {
  size_ = size;
}

unsigned char* airmap::mavlink::boost::UdpChannel::EncodedBuffer::data() {
  return data_.data();
}

const unsigned char* airmap::mavlink::boost::UdpChannel::EncodedBuffer::data() const {
  return data_.data();
}

void airmap::mavlink::boost::UdpChannel::send_impl(const mavlink_message_t& message) {

    // const mavlink_message_t& message;
    // std::array<char, 1> send_buf  = { 0 };
    // ::boost::asio::ip::udp::endpoint endpoint;
    // socket_.send_to(::boost::asio::buffer(send_buf), endpoint);
  log_.infof(component, "TEST");
  if (state_ == 1)
    log_.infof(component, "%s:%d", endpoint_.address().to_string(), endpoint_.port());
  // log_.debugf(component, "processing mavlink message for session on endpoint %s:%d",
  //             socket_.remote_endpoint().address().to_string(), socket_.remote_endpoint().port());

  EncodedBuffer eb;
  eb.set_size(mavlink_msg_to_send_buffer(eb.data(), &message));

  io_service_->post([sp = shared_from_this(), eb = std::move(eb)]() {
    sp->buffers_.emplace(eb);
    if (sp->buffers_.size() == 1)
      sp->process();
  });
}

void airmap::mavlink::boost::UdpChannel::process() {
  const auto& eb = buffers_.front();

::boost::asio::ip::udp::endpoint destination(
    ::boost::asio::ip::address::from_string("0.0.0.0"), 14540);
log_.infof(component, "2222222");

  socket_.async_send_to(::boost::asio::buffer(eb.data(), eb.size()), destination,
                             [sp = shared_from_this()](const auto& error, auto) {
                               sp->buffers_.pop();

                               if (error) {
                                 sp->log_.infof(component,
                                                "failed to process mavlink message for session on endpoint %s:%d: %s",
                                                sp->socket_.remote_endpoint().address().to_string(),
                                                sp->socket_.remote_endpoint().port(), error.message());

                                 return;
                               }

                               if (!sp->buffers_.empty())
                                 sp->process();
                             });
}

void airmap::mavlink::boost::UdpChannel::handle_read(const ::boost::system::error_code& ec, std::size_t transferred) {
  if (ec) {
    log_.errorf(component, "failed to read from tcp endpoint: %s", ec.message());
    return;
  }

  if (auto result = process_mavlink_data(buffer_.begin(), buffer_.begin() + transferred))
    invoke_subscribers(result.get());

  start_impl();
}
