set(SOURCES
    core/order_book.cpp
    core/matching_engine.cpp
    memory/pool_allocator.cpp
    main_working.cpp
)

add_executable(nanotrader ${SOURCES})

target_link_libraries(nanotrader ${COMMON_LIBRARIES})

target_compile_features(nanotrader PRIVATE cxx_std_20)