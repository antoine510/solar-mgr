
########
# Solar MPPTs manager
########

set(SOURCE_FILES
	MPPTData.h

	RS485Serial.hpp
	RS485Serial.cpp

	solar-mgr.cpp
)
add_executable(solar-mgr ${SOURCE_FILES})

install(TARGETS solar-mgr RUNTIME)
