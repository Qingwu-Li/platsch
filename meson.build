project('platsch', 'c')

platsch_dep = [dependency('libdrm', required: true)]
sources = ['libplatsch.c']


libplatsch = static_library('libplatsch',
    sources,
    dependencies: platsch_dep,
)

executable('platsch',
    'platsch.c',
    dependencies: platsch_dep,
    link_with: libplatsch,
    install: true,
    include_directories: include_directories('.')
)
