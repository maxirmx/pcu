#include "fuelflux/OfflineQueue.h"

namespace fuelflux {

void OfflineQueue::enqueue(Item&& it) {
    std::lock_guard<std::mutex> lock(mtx_);
    q_.emplace(std::move(it));
}

bool OfflineQueue::empty() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return q_.empty();
}

bool OfflineQueue::try_pop(Item& out) {
    std::lock_guard<std::mutex> lock(mtx_);
    if(q_.empty()) return false;
    out = std::move(q_.front());
    q_.pop();
    return true;
}

} // namespace fuelflux
