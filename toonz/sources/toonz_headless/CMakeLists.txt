set(CMAKE_AUTOMOC ON)

add_executable(toonz_headless
    main.cpp
)

target_link_libraries(toonz_headless
    Qt5::Core
    Qt5::Gui
    Qt5::Script
    toonzlib
    image
    sound
)

# GNU linkers may otherwise discard plug-in libraries whose symbols are
# discovered dynamically. These flags are not valid for MSVC or Apple ld.
if(UNIX AND NOT APPLE)
    target_link_libraries(toonz_headless
        -Wl,--no-as-needed
        tnzstdfx
        colorfx
        -Wl,--as-needed
    )
else()
    target_link_libraries(toonz_headless
        tnzstdfx
        colorfx
    )
endif()
