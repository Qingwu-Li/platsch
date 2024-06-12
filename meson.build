project('platsch', 'c', version: '2024.06.0')

dep_libdrm = dependency('libdrm', version : '>= 2.4.112', required : true)
sources = ['libplatsch.c']


libplatsch = static_library('libplatsch',
    sources,
    dependencies: dep_libdrm,
    install: true,
    include_directories: include_directories('.')
)

# Create the spinner executable if SPINNER true
if get_option('SPINNER')
    spinner_dep = [
        dependency('cairo', required: true),
        dependency('libdrm', required: true)
    ]

    spinner_src = [
        'spinner.c',
        'spinner_conf.c'
    ]

    args = ['-DSPINNER']

    executable('spinner',
        spinner_src,
        dependencies: spinner_dep,
        link_with: libplatsch,
        c_args: args,
        install: true,
        include_directories: include_directories('.')
    )
endif
