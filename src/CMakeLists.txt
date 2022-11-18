
########
# Solar MPPTs manager
########

set(SOURCE_FILES
	MPPTData.h

	SolarSerial.hpp
	SolarSerial.cpp

	solar-mgr.cpp
)
add_executable(solar-mgr ${SOURCE_FILES})

install(TARGETS solar-mgr RUNTIME)
