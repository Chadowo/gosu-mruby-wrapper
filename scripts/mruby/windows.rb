MRuby::Build.new do |conf|
  conf.toolchain :gcc

  # Include gems
  conf.gembox File.expand_path('gosu', File.dirname(__FILE__))

  # C compiler settings
  conf.cc.defines += %w[MRB_INT64]
  conf.cc.flags += %w[-fpermissive -O3]

  linker.libraries.push('ws2_32')

  conf.enable_debug # Won't compile otherwise
  conf.enable_cxx_abi
end
