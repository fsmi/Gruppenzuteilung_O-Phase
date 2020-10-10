include(FetchContent)
FetchContent_Populate(
  boost-src
  URL http://downloads.sourceforge.net/project/boost/boost/1.74.0/boost_1_74_0.tar.bz2
  URL_HASH MD5=da07ca30dd1c0d1fdedbd487efee01bd
  SOURCE_DIR boost
)
