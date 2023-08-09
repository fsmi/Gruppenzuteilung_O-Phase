include(FetchContent)
FetchContent_Populate(
  boost-src
  URL https://sourceforge.net/projects/boost/files/boost/1.82.0/boost_1_82_0.tar.bz2/download
  URL_HASH MD5=b45dac8b54b58c087bfbed260dbfc03a
  SOURCE_DIR boost
)
