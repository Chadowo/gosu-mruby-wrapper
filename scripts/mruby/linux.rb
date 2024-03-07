MRuby::Build.new do |conf|
  conf.toolchain

  # Include gems
  conf.gembox File.expand_path('gosu', File.dirname(__FILE__))

  # C compiler settings
  conf.cc.defines += %w[MRB_INT64]
  conf.cc.flags += %w[-fpermissive -O3]

  conf.enable_cxx_abi
end
