# x264-android-enc264


本示例演示了在android下使用X264编码从摄像头取得的视频流并保存为文件

本工程在Windows下使用eclipse+ndk+Cygwin编译

保存的文件使用Elecard分析正常


# X264编译

请查看 x264_build_android.sh 


#demo

使用摄像头采集视频数据，jni接口调用底层的x264进行编码存储

效率：高通cpu 650，只测试过720P，30帧，4M码流 OK.

本来要通过互联网传输，320*240 / 640*480  在512k的码流下表现较差。



#THANKS

资料均取之于网络，对提供知识的人表示感谢！
