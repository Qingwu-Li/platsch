project('platsch', 'c', version: '2024.06.0', license : '0BSD')

dep_libdrm = dependency('libdrm',
    version: '>= 2.4.112',
    static: true
)

libplasch_sources = ['libplatsch.c']

libplatsch = static_library('platsch',
    libplasch_sources,
    dependencies: dep_libdrm
)

platsch_sources = ['platsch.c']

executable('platsch',
    platsch_sources,
    link_with: libplatsch,
    dependencies: dep_libdrm,
    link_args: '-static',
    install: true,
    install_dir : 'sbin'
)


if get_option('SPINNER')
    spinner_dep = [
        dependency('cairo', version: '>= 1.0', static: true),
        dep_libdrm,
    ]

    spinner_src = [
        'spinner.c',
        'spinner_conf.c'
    ]

    executable('spinner',
        spinner_src,
        dependencies: spinner_dep,
        link_with: libplatsch,
        install: true,
        install_dir : 'sbin'
    )
endif
