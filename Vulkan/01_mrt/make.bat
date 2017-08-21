del main.exe
cl main.cpp win.cpp /EHsc /Ox /IC:\VulkanSDK\1.0.49.0\Include user32.lib gdi32.lib C:\VulkanSDK\1.0.49.0\Lib\vulkan-1.lib
REM cl main.cpp win.cpp /EHsc /Ox  user32.lib gdi32.lib vulkan-1.lib
main
