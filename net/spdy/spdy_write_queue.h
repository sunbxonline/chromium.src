// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_SPDY_SPDY_WRITE_QUEUE_H_
#define NET_SPDY_SPDY_WRITE_QUEUE_H_

#include <deque>

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "net/base/net_export.h"
#include "net/base/request_priority.h"
#include "net/spdy/spdy_protocol.h"

namespace net {

class SpdyFrameProducer;
class SpdyStream;

// A queue of SpdyFrameProducers to produce frames to write. Ordered
// by priority, and then FIFO.
class NET_EXPORT_PRIVATE SpdyWriteQueue {
 public:
  SpdyWriteQueue();
  ~SpdyWriteQueue();

  // Enqueues the given frame producer of the given type at the given
  // priority associated with the given stream, which may be NULL if
  // the frame producer is not associated with a stream. If |stream|
  // is non-NULL, its priority must be equal to |priority|.
  void Enqueue(RequestPriority priority,
               SpdyFrameType frame_type,
               scoped_ptr<SpdyFrameProducer> frame_producer,
               const scoped_refptr<SpdyStream>& stream);

  // Dequeues the frame producer with the highest priority that was
  // enqueued the earliest and its associated stream. Returns true and
  // fills in |frame_type|, |frame_producer|, and |stream| if
  // successful -- otherwise, just returns false.
  bool Dequeue(SpdyFrameType* frame_type,
               scoped_ptr<SpdyFrameProducer>* frame_producer,
               scoped_refptr<SpdyStream>* stream);

  // Removes all pending writes for the given stream, which must be
  // non-NULL.
  void RemovePendingWritesForStream(const scoped_refptr<SpdyStream>& stream);

  // Removes all pending writes.
  void Clear();

 private:
  // A struct holding a frame producer and its associated stream.
  struct PendingWrite {
    SpdyFrameType frame_type;
    // This has to be a raw pointer since we store this in an STL
    // container.
    SpdyFrameProducer* frame_producer;
    scoped_refptr<SpdyStream> stream;

    PendingWrite();
    PendingWrite(SpdyFrameType frame_type,
                 SpdyFrameProducer* frame_producer,
                 const scoped_refptr<SpdyStream>& stream);
    ~PendingWrite();
  };

  // The actual write queue, binned by priority.
  std::deque<PendingWrite> queue_[NUM_PRIORITIES];

  DISALLOW_COPY_AND_ASSIGN(SpdyWriteQueue);
};

}  // namespace net

#endif  // NET_SPDY_SPDY_WRITE_QUEUE_H_
