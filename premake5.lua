-- premake5.lua

workspace "VectorC"
   configurations { "Debug", "Release" }
   platforms { "x64" }   
   startproject "VectorC"
   includedirs { "." }
   defines { "_CRT_SECURE_NO_WARNINGS" }
   
filter { "action:gmake*" } --, "system:macosx" }
   architecture "ARM64"

filter {}

project "VectorC"
   location "VectorC"
   targetname "vecc"
   kind "ConsoleApp"
--   architecture "x86_64"  
   language "C"
   buildoptions { "-std=c17" } ---arch x86_64" } --, "-target x86_64-apple-darwin" } -- -gno-column-info -ffast-math -Xclang -flto-visibility-public-std" }
   linkoptions { "-g -gno-column-info" } -- -arch x86_64" } --, "-target x86_64-apple-darwin" } 
   targetdir "bin/%{cfg.buildcfg}"

   files 
   { 
      "VectorC/**.c", 
      "VectorC/**.h", 
   }

   excludes
   {
   }

   filter "configurations:Debug"
      defines { "_DEBUG" }
      optimize "Off"
      symbols  "Full"      

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "Full"
