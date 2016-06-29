#APP_STL := stlport_static

#使用stlport_static选项时，与boost一起使用，会在链接时产生如下错误：
# undefined reference to 'std::basic_string<char, std::char_traits<char>, std::allocator<char> >::~basic_string()'  
APP_STL := gnustl_static #stlport_static

