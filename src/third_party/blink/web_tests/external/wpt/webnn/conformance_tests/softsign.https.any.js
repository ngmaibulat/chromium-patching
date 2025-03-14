// META: title=test WebNN API softsign operation
// META: global=window
// META: variant=?cpu
// META: variant=?gpu
// META: variant=?npu
// META: script=../resources/utils.js
// META: timeout=long

'use strict';

// https://www.w3.org/TR/webnn/#api-mlgraphbuilder-softsign-method
// Compute the softsign function of the input tensor. The calculation follows
// the expression x / (1 + |x|).
//
// MLOperand softsign(MLOperand input);

const softsignTests = [
  {
    'name': 'softsign positive float32 1D constant tensor',
    'graph': {
      'inputs': {
        'softsignInput': {
          'data': [
            1.5834133625030518, 4.078719139099121, 8.883357048034668,
            8.070859909057617,  8.211773872375488, 2.4554004669189453,
            0.653374195098877,  7.866281032562256, 3.123955249786377,
            8.013792037963867,  3.940986156463623, 1.813172698020935,
            2.3906760215759277, 1.335968017578125, 9.416410446166992,
            0.4432569146156311, 5.236661911010742, 9.42424201965332,
            7.816190242767334,  5.849185943603516, 8.780370712280273,
            5.120515823364258,  7.117222309112549, 4.599106788635254
          ],
          'descriptor': {shape: [24], dataType: 'float32'},
          'constant': true
        }
      },
      'operators': [{
        'name': 'softsign',
        'arguments': [{'input': 'softsignInput'}],
        'outputs': 'softsignOutput'
      }],
      'expectedOutputs': {
        'softsignOutput': {
          'data': [
            0.6129152178764343,  0.8030999898910522, 0.8988198041915894,
            0.8897568583488464,  0.8914432525634766, 0.7105979323387146,
            0.3951762318611145,  0.8872131109237671, 0.7575143575668335,
            0.8890588879585266,  0.7976112365722656, 0.6445294618606567,
            0.7050735354423523,  0.5719119310379028, 0.9039976596832275,
            0.30712267756462097, 0.8396578431129456, 0.9040697813034058,
            0.8865723013877869,  0.8539972305297852, 0.8977543711662292,
            0.8366150856018066,  0.8768051266670227, 0.8214001059532166
          ],
          'descriptor': {shape: [24], dataType: 'float32'}
        }
      }
    }
  },
  {
    'name': 'softsign positive float32 0D tensor',
    'graph': {
      'inputs': {
        'softsignInput': {
          'data': [1.5834133625030518],
          'descriptor': {shape: [], dataType: 'float32'}
        }
      },
      'operators': [{
        'name': 'softsign',
        'arguments': [{'input': 'softsignInput'}],
        'outputs': 'softsignOutput'
      }],
      'expectedOutputs': {
        'softsignOutput': {
          'data': [0.6129152178764343],
          'descriptor': {shape: [], dataType: 'float32'}
        }
      }
    }
  },
  {
    'name': 'softsign negative float32 0D tensor',
    'graph': {
      'inputs': {
        'softsignInput': {
          'data': [-2.597844123840332],
          'descriptor': {shape: [], dataType: 'float32'}
        }
      },
      'operators': [{
        'name': 'softsign',
        'arguments': [{'input': 'softsignInput'}],
        'outputs': 'softsignOutput'
      }],
      'expectedOutputs': {
        'softsignOutput': {
          'data': [-0.7220557928085327],
          'descriptor': {shape: [], dataType: 'float32'}
        }
      }
    }
  },
  {
    'name': 'softsign positive float32 1D tensor',
    'graph': {
      'inputs': {
        'softsignInput': {
          'data': [
            1.5834133625030518, 4.078719139099121, 8.883357048034668,
            8.070859909057617,  8.211773872375488, 2.4554004669189453,
            0.653374195098877,  7.866281032562256, 3.123955249786377,
            8.013792037963867,  3.940986156463623, 1.813172698020935,
            2.3906760215759277, 1.335968017578125, 9.416410446166992,
            0.4432569146156311, 5.236661911010742, 9.42424201965332,
            7.816190242767334,  5.849185943603516, 8.780370712280273,
            5.120515823364258,  7.117222309112549, 4.599106788635254
          ],
          'descriptor': {shape: [24], dataType: 'float32'}
        }
      },
      'operators': [{
        'name': 'softsign',
        'arguments': [{'input': 'softsignInput'}],
        'outputs': 'softsignOutput'
      }],
      'expectedOutputs': {
        'softsignOutput': {
          'data': [
            0.6129152178764343,  0.8030999898910522, 0.8988198041915894,
            0.8897568583488464,  0.8914432525634766, 0.7105979323387146,
            0.3951762318611145,  0.8872131109237671, 0.7575143575668335,
            0.8890588879585266,  0.7976112365722656, 0.6445294618606567,
            0.7050735354423523,  0.5719119310379028, 0.9039976596832275,
            0.30712267756462097, 0.8396578431129456, 0.9040697813034058,
            0.8865723013877869,  0.8539972305297852, 0.8977543711662292,
            0.8366150856018066,  0.8768051266670227, 0.8214001059532166
          ],
          'descriptor': {shape: [24], dataType: 'float32'}
        }
      }
    }
  },
  {
    'name': 'softsign negative float32 1D tensor',
    'graph': {
      'inputs': {
        'softsignInput': {
          'data': [
            -2.597844123840332,  -0.4449555575847626, -9.095475196838379,
            -3.7480077743530273, -1.3867290019989014, -8.220329284667969,
            -3.538342237472534,  -9.364588737487793,  -6.283252239227295,
            -5.002012252807617,  -8.245729446411133,  -3.775470495223999,
            -4.087255001068115,  -7.381676197052002,  -5.8829216957092285,
            -8.338910102844238,  -6.60154914855957,   -4.491941928863525,
            -3.5247786045074463, -4.43991231918335,   -5.234262466430664,
            -1.5911732912063599, -9.106277465820312,  -8.523774147033691
          ],
          'descriptor': {shape: [24], dataType: 'float32'}
        }
      },
      'operators': [{
        'name': 'softsign',
        'arguments': [{'input': 'softsignInput'}],
        'outputs': 'softsignOutput'
      }],
      'expectedOutputs': {
        'softsignOutput': {
          'data': [
            -0.7220557928085327, -0.3079372048377991, -0.9009457230567932,
            -0.7893853783607483, -0.5810165405273438, -0.891543984413147,
            -0.7796552181243896, -0.9035176634788513, -0.8626986742019653,
            -0.8333892226219177, -0.8918419480323792, -0.7905965447425842,
            -0.8034303188323975, -0.8806921243667603, -0.8547128438949585,
            -0.8929211497306824, -0.8684478402137756, -0.8179150223731995,
            -0.7789947390556335, -0.8161734938621521, -0.8395960927009583,
            -0.6140744686126709, -0.9010515809059143, -0.894999623298645
          ],
          'descriptor': {shape: [24], dataType: 'float32'}
        }
      }
    }
  },
  {
    'name': 'softsign float32 2D tensor',
    'graph': {
      'inputs': {
        'softsignInput': {
          'data': [
            -8.343496322631836,  -6.920152187347412,  2.699638843536377,
            -8.663105010986328,  -3.1905343532562256, 7.657886981964111,
            6.650215148925781,   6.058011054992676,   0.6634320616722107,
            5.8058037757873535,  -0.32821124792099,   1.2704304456710815,
            -9.946120262145996,  6.905375003814697,   -0.031071536242961884,
            -3.9696409702301025, 6.270823001861572,   -2.639260768890381,
            3.0513505935668945,  7.426476955413818,   -8.454667091369629,
            7.135868072509766,   -4.986093997955322,  -7.859614849090576
          ],
          'descriptor': {shape: [4, 6], dataType: 'float32'}
        }
      },
      'operators': [{
        'name': 'softsign',
        'arguments': [{'input': 'softsignInput'}],
        'outputs': 'softsignOutput'
      }],
      'expectedOutputs': {
        'softsignOutput': {
          'data': [
            -0.8929736614227295, -0.8737397789955139,  0.7297033667564392,
            -0.8965135812759399, -0.7613669633865356,  0.8844983577728271,
            0.8692847490310669,  0.8583170175552368,   0.3988332748413086,
            0.8530665636062622,  -0.24710771441459656, 0.5595548748970032,
            -0.9086434245109558, 0.8735038042068481,   -0.03013519011437893,
            -0.798778235912323,  0.8624640107154846,   -0.7252188920974731,
            0.7531687617301941,  0.88132643699646,     -0.8942321538925171,
            0.8770874738693237,  -0.8329461812973022,  -0.8871282935142517
          ],
          'descriptor': {shape: [4, 6], dataType: 'float32'}
        }
      }
    }
  },
  {
    'name': 'softsign float32 3D tensor',
    'graph': {
      'inputs': {
        'softsignInput': {
          'data': [
            -8.343496322631836,  -6.920152187347412,  2.699638843536377,
            -8.663105010986328,  -3.1905343532562256, 7.657886981964111,
            6.650215148925781,   6.058011054992676,   0.6634320616722107,
            5.8058037757873535,  -0.32821124792099,   1.2704304456710815,
            -9.946120262145996,  6.905375003814697,   -0.031071536242961884,
            -3.9696409702301025, 6.270823001861572,   -2.639260768890381,
            3.0513505935668945,  7.426476955413818,   -8.454667091369629,
            7.135868072509766,   -4.986093997955322,  -7.859614849090576
          ],
          'descriptor': {shape: [2, 3, 4], dataType: 'float32'}
        }
      },
      'operators': [{
        'name': 'softsign',
        'arguments': [{'input': 'softsignInput'}],
        'outputs': 'softsignOutput'
      }],
      'expectedOutputs': {
        'softsignOutput': {
          'data': [
            -0.8929736614227295, -0.8737397789955139,  0.7297033667564392,
            -0.8965135812759399, -0.7613669633865356,  0.8844983577728271,
            0.8692847490310669,  0.8583170175552368,   0.3988332748413086,
            0.8530665636062622,  -0.24710771441459656, 0.5595548748970032,
            -0.9086434245109558, 0.8735038042068481,   -0.03013519011437893,
            -0.798778235912323,  0.8624640107154846,   -0.7252188920974731,
            0.7531687617301941,  0.88132643699646,     -0.8942321538925171,
            0.8770874738693237,  -0.8329461812973022,  -0.8871282935142517
          ],
          'descriptor': {shape: [2, 3, 4], dataType: 'float32'}
        }
      }
    }
  },
  {
    'name': 'softsign float32 4D tensor',
    'graph': {
      'inputs': {
        'softsignInput': {
          'data': [
            -8.343496322631836,  -6.920152187347412,  2.699638843536377,
            -8.663105010986328,  -3.1905343532562256, 7.657886981964111,
            6.650215148925781,   6.058011054992676,   0.6634320616722107,
            5.8058037757873535,  -0.32821124792099,   1.2704304456710815,
            -9.946120262145996,  6.905375003814697,   -0.031071536242961884,
            -3.9696409702301025, 6.270823001861572,   -2.639260768890381,
            3.0513505935668945,  7.426476955413818,   -8.454667091369629,
            7.135868072509766,   -4.986093997955322,  -7.859614849090576
          ],
          'descriptor': {shape: [1, 2, 3, 4], dataType: 'float32'}
        }
      },
      'operators': [{
        'name': 'softsign',
        'arguments': [{'input': 'softsignInput'}],
        'outputs': 'softsignOutput'
      }],
      'expectedOutputs': {
        'softsignOutput': {
          'data': [
            -0.8929736614227295, -0.8737397789955139,  0.7297033667564392,
            -0.8965135812759399, -0.7613669633865356,  0.8844983577728271,
            0.8692847490310669,  0.8583170175552368,   0.3988332748413086,
            0.8530665636062622,  -0.24710771441459656, 0.5595548748970032,
            -0.9086434245109558, 0.8735038042068481,   -0.03013519011437893,
            -0.798778235912323,  0.8624640107154846,   -0.7252188920974731,
            0.7531687617301941,  0.88132643699646,     -0.8942321538925171,
            0.8770874738693237,  -0.8329461812973022,  -0.8871282935142517
          ],
          'descriptor': {shape: [1, 2, 3, 4], dataType: 'float32'}
        }
      }
    }
  },
  {
    'name': 'softsign float32 5D tensor',
    'graph': {
      'inputs': {
        'softsignInput': {
          'data': [
            -8.343496322631836,  -6.920152187347412,  2.699638843536377,
            -8.663105010986328,  -3.1905343532562256, 7.657886981964111,
            6.650215148925781,   6.058011054992676,   0.6634320616722107,
            5.8058037757873535,  -0.32821124792099,   1.2704304456710815,
            -9.946120262145996,  6.905375003814697,   -0.031071536242961884,
            -3.9696409702301025, 6.270823001861572,   -2.639260768890381,
            3.0513505935668945,  7.426476955413818,   -8.454667091369629,
            7.135868072509766,   -4.986093997955322,  -7.859614849090576
          ],
          'descriptor': {shape: [1, 2, 1, 3, 4], dataType: 'float32'}
        }
      },
      'operators': [{
        'name': 'softsign',
        'arguments': [{'input': 'softsignInput'}],
        'outputs': 'softsignOutput'
      }],
      'expectedOutputs': {
        'softsignOutput': {
          'data': [
            -0.8929736614227295, -0.8737397789955139,  0.7297033667564392,
            -0.8965135812759399, -0.7613669633865356,  0.8844983577728271,
            0.8692847490310669,  0.8583170175552368,   0.3988332748413086,
            0.8530665636062622,  -0.24710771441459656, 0.5595548748970032,
            -0.9086434245109558, 0.8735038042068481,   -0.03013519011437893,
            -0.798778235912323,  0.8624640107154846,   -0.7252188920974731,
            0.7531687617301941,  0.88132643699646,     -0.8942321538925171,
            0.8770874738693237,  -0.8329461812973022,  -0.8871282935142517
          ],
          'descriptor': {shape: [1, 2, 1, 3, 4], dataType: 'float32'}
        }
      }
    }
  }
];

if (navigator.ml) {
  softsignTests.forEach((test) => {
    webnn_conformance_test(buildAndExecuteGraph, getPrecisionTolerance, test);
  });
} else {
  test(() => assert_implements(navigator.ml, 'missing navigator.ml'));
}
