# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS src/main.c	src/display.c src/http_server.c src/io_util.c src/wifi_ap.c src/user_config.c        # list the source files of this component
    INCLUDE_DIRS "inc"        # optional, add here public include directories
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)
