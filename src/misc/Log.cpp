#include <mutex>

using std::mutex;

// This lock must be acquired before writing to stdout/stderror is performed
mutex outputLock;
