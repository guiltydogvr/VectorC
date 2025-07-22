-- premake5.lua

workspace "VectorC"
   configurations { "Debug", "Release" }
   platforms { "x64" }   
   startproject "VectorC"
   includedirs { "." }


filter { "action:vs*" }
   system "windows"
   architecture "x86_64"  
   vectorextensions "SSE4.1"
   toolset "msc-clangcl"
   buildoptions { "-march=native" } 

filter { "action:xcode*", "platforms:x64" }
   system "macosx"
   architecture "x86_64"  
   vectorextensions "SSE4.1"
   toolset "clang"
   defines("MACOS")    
   buildoptions { "-std=c17" } ---arch x86_64" } --, "-target x86_64-apple-darwin" } -- -gno-column-info -ffast-math -Xclang -flto-visibility-public-std" }
   linkoptions { "-g -gno-column-info" } -- -arch x86_64" } --, "-target x86_64-apple-darwin" }  

   xcodebuildsettings
   {
      ["GCC_C_LANGUAGE_STANDARD"] = "c11";         
      ["CLANG_CXX_LANGUAGE_STANDARD"] = "c++11";
      ["CLANG_CXX_LIBRARY"]  = "libc++";
      ["SDKROOT"] = "macosx";
      ["CLANG_ENABLE_OBJC_WEAK"] = "YES";
      ["CODE_SIGN_IDENTITY"] = "-";                      --iphoneos";  
      ["VALIDATE_WORKSPACE_SKIPPED_SDK_FRAMEWORKS"] = "OpenGL";
      ["CLANG_WARN_BLOCK_CAPTURE_AUTORELEASING"] = "YES";
      ["CLANG_WARN_BOOL_CONVERSION"] = "YES";
      ["CLANG_WARN_COMMA"] = "YES";
      ["CLANG_WARN_CONSTANT_CONVERSION"] = "YES";
      ["CLANG_WARN_DEPRECATED_OBJC_IMPLEMENTATIONS"] = "YES";
      ["CLANG_WARN_EMPTY_BODY"] = "YES";
      ["CLANG_WARN_ENUM_CONVERSION"] = "YES";
      ["CLANG_WARN_INFINITE_RECURSION"] = "YES";
      ["CLANG_WARN_INT_CONVERSION"] = "YES";
      ["CLANG_WARN_NON_LITERAL_NULL_CONVERSION"] = "YES";
      ["CLANG_WARN_OBJC_IMPLICIT_RETAIN_SELF"] = "YES";
      ["CLANG_WARN_OBJC_LITERAL_CONVERSION"] = "YES";
      ["CLANG_WARN_QUOTED_INCLUDE_IN_FRAMEWORK_HEADER"] = "YES";
      ["CLANG_WARN_RANGE_LOOP_ANALYSIS"] = "YES";
      ["CLANG_WARN_STRICT_PROTOTYPES"] = "NO"; --"YES";
      ["CLANG_WARN_SUSPICIOUS_MOVE"] = "YES";
      ["CLANG_WARN_UNREACHABLE_CODE"] = "YES";
      ["CLANG_WARN__DUPLICATE_METHOD_MATCH"] = "YES";
      ["GCC_WARN_64_TO_32_BIT_CONVERSION"] = "NO"; --"YES";
      ["GCC_WARN_UNDECLARED_SELECTOR"] = "YES";
      ["GCC_WARN_UNINITIALIZED_AUTOS"] = "YES";
      ["GCC_WARN_UNUSED_FUNCTION"] = "YES";
      ["GCC_NO_COMMON_BLOCKS"] = "YES";
      ["ENABLE_STRICT_OBJC_MSGSEND"] = "YES";
   }   

filter { "action:xcode*", "configurations:Debug" }
   xcodebuildsettings
   {
      ["ENABLE_TESTABILITY"] = "YES";
   } 


filter { "action:gmake*" } --, "system:macosx" }
   architecture "ARM64"

filter {}

project "VectorC"
   location "VectorC"
   targetname "vecc"
   kind "ConsoleApp"
   architecture "x86_64"  
   language "C"
   targetdir "bin/%{cfg.buildcfg}"
   defines("_CRT_SECURE_NO_WARNINGS")    

   files 
   { 
      "VectorC/**.c", 
      "VectorC/**.h", 
   }

   excludes
   {
   }

 filter { "action:vs*" }
   defines("WIN64", "_CRT_NONSTDC_NO_DEPRECATE", "_CRT_NONSTDC_NO_WARNINGS")    
   buildoptions { "-march=native" } 
  
   filter "configurations:Debug"
      defines { "_DEBUG" }
      optimize "Off"
      symbols  "Full"      

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "Full"
