#include <iostream>

#include <CL/sycl.hpp>
#include <CL/sycl/usm.hpp>

using namespace cl::sycl;

int main(int argc, char **argv) {
    const int N = 32;
    int i;

    std::string arg_device_type;
    if (argc < 2) {
        arg_device_type = "gpu";
    } else {
        arg_device_type = argv[1];
    }

    queue q;
    if (arg_device_type == "cpu") {
        q = queue( cpu_selector() );
    } else if (arg_device_type == "gpu") {
        q = queue( gpu_selector() );
    } else if (arg_device_type == "host") {
        q = queue( host_selector() );
    } else {
        std::cout << "Usage: " << argv[0] << " cpu|gpu|host" << std::endl;
        return 1;
    }

    auto dev = q.get_device();
    std::string type;
    if (dev.is_cpu()) {
        type = "CPU  ";
    } else if (dev.is_gpu()) {
        type = "GPU  ";
    } else if (dev.is_host()) {
        type = "HOST ";
    } else {
        type = "OTHER";
    }
    std::cout << "[" << type << "] "
              << dev.get_info<info::device::name>()
              << " {" << dev.get_info<info::device::vendor>() << "}"
              << std::endl;

    // test sycl usm C based api
    int *h_a_ptr = static_cast<int *>(malloc(N*sizeof(int)));

    int *d_a_ptr = static_cast<int *>(sycl::malloc_device(N*sizeof(int), q));
    if (d_a_ptr == nullptr) {
        std::cout << "Error: unable to allocat device memory" << std::endl;
        return 1;
    }

    auto event = q.submit([&](handler & cgh) {
        cgh.parallel_for<class FillVector>(range<1>(N), [=](id<1> idx) {
            d_a_ptr[idx] = idx*idx;
        });
    });

    q.memcpy(h_a_ptr, d_a_ptr, N*sizeof(int));
    q.wait();

    for (i=0; i<N; i++) {
        std::cout << i << ": " << h_a_ptr[i] << std::endl;
    }

    sycl::free(d_a_ptr, q);
    free(h_a_ptr);

    return 0;
}