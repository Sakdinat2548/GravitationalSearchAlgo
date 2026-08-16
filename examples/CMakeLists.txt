add_executable(main main.cpp)
target_link_libraries(main PRIVATE gsa::gsa)
target_compile_options(main PRIVATE -Wall -Wextra)
