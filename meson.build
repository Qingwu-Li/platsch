project('platsch', 'c', version: '2024.06.0')

dep_libdrm = dependency('libdrm', version : '>= 2.4.112', required : true)
sources = ['libplatsch.c']


libplatsch = static_library('libplatsch',
    sources,
    dependencies: dep_libdrm,
    install: true,
    include_directories: include_directories('.')
)
