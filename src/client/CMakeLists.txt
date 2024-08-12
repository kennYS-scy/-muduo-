#定义一个SRC_LIST，包含该目录下的所有源文件
aux_source_directory(. SRC_LIST)

#通过SRCLIST的源文件生成可执行文件ChatServer
add_executable(ChatClient ${SRC_LIST})

#指定可执行文件连接时需要依赖的库文件
target_link_libraries(ChatClient pthread)
