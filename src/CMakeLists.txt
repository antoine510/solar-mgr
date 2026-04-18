
########
# Solar MPPTs and battery current sensors manager
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
target_link_libraries(solar-mgr PRIVATE nlohmann_json::nlohmann_json)

install(TARGETS solar-mgr RUNTIME)
