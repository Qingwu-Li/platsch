project('platsch', 'c', version: '2024.06.0', license : '0BSD')

dep_libdrm = dependency('libdrm',
    version: '>= 2.4.112',
    static: true
)

executable('platsch',
    'platsch.c',
    dependencies: dep_libdrm,
    link_args: '-static',
    install: true,
    install_dir : 'sbin'
)
