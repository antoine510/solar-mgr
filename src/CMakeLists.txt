
########
# Solar MPPTs manager
########

set(SOURCE_FILES
	Bus.hpp
	Bus.cpp

	Modules/BusModule.hpp
	
	Modules/CurrentSensor.hpp
	Modules/MPPT.hpp

	Network.hpp
	Network.cpp

	solar-mgr.cpp
)
add_executable(solar-mgr ${SOURCE_FILES})

install(TARGETS solar-mgr RUNTIME)
