project(EasyGraphics)
set(SRC ./system/SystemIO.cc)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_COMPILER arm-none-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-none-linux-gnueabihf-g++)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

include_directories(.)

add_executable(Quickstart ./example/Quickstart.cc ${SRC})
add_executable(EliminatingGame ./example/EliminatingGame.cc ${SRC})
add_executable(Calculator ./example/Calculator.cc ${SRC})
add_executable(CalculatorAnimation ./example/CalculatorAnimation.cc ${SRC})
add_executable(CalculatorIm ./example/CalculatorIm.cc ${SRC})

