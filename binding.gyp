{
  "targets": [
    {
      "target_name": "pv_porcupine",
      "sources": [ "src/porcupine.cc" ],
      "include_dirs": [
          "<!(node -e \"require('nan')\")",
          "Porcupine/include"
      ],
      "conditions" : [
        [
          'OS=="mac"', {
            'xcode_settings': {
              'GCC_ENABLE_CPP_RTTI': 'YES',
              'MACOSX_DEPLOYMENT_TARGET': '10.13',
              'OTHER_CPLUSPLUSFLAGS': [
                '-std=c++11',
                '-stdlib=libc++',
                '-fexceptions'
              ]
            },
            "link_settings": {
              "libraries": [
                "<@(module_root_dir)/Porcupine/lib/mac/x86_64/libpv_porcupine.a"
              ]
            }
          },
        ],
        [
          'OS=="linux"', {
            "conditions": [
              ['target_arch=="arm"', {
                "link_settings": {
                  "libraries": [
                    "<@(module_root_dir)/Porcupine/lib/raspberry-pi/cortex-a53/libpv_porcupine.a"
                  ]
                }
              }],
              ['target_arch=="ia32"', {
                "link_settings": {
                  "libraries": [
                    "<@(module_root_dir)/Porcupine/lib/linux/i386/libpv_porcupine.a"
                  ]
                }
              }],
              ['target_arch=="x64"', {
                "link_settings": {
                  "libraries": [
                    "<@(module_root_dir)/Porcupine/lib/linux/x86_64/libpv_porcupine.a"
                  ]
                }
              }]
            ]
          }
        ]
      ]
    }
  ]
}
