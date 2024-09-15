
########
# Solar MPPTs manager
########

set(SOURCE_FILES
	Bus.hpp
	Bus.cpp

	BusModule.hpp
	CurrentSensor.hpp

	solar-mgr.cpp
)
add_executable(solar-mgr ${SOURCE_FILES})

install(TARGETS solar-mgr RUNTIME)
