// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import type {ColorOption, DpiOption, DuplexOption, PrintPreviewModelElement, PrintTicket, RecentDestination, Settings} from 'chrome://print/print_preview.js';
import {
  Destination, DestinationOrigin, DuplexMode,
  makeRecentDestination, MarginsType,
  PrinterType, ScalingType, Size} from 'chrome://print/print_preview.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {flush} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {assertDeepEquals, assertEquals, assertFalse, assertNotEquals, assertTrue} from 'chrome://webui-test/chai_assert.js';
import {eventToPromise} from 'chrome://webui-test/test_util.js';

import {
  getCddTemplateWithAdvancedSettings} from './print_preview_test_utils.js';


function assertMarginsSettingsResetToDefault(settings: Settings) {
  assertEquals(settings.margins.value, MarginsType.DEFAULT);
  assertFalse('marginTop' in settings.customMargins.value);
  assertFalse('marginRight' in settings.customMargins.value);
  assertFalse('marginBottom' in settings.customMargins.value);
  assertFalse('marginLeft' in settings.customMargins.value);
}

suite('ModelTest', function() {
  let model: PrintPreviewModelElement;

  setup(function() {
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    model = document.createElement('print-preview-model');
    document.body.appendChild(model);
  });

  /**
   * Tests state restoration with all boolean settings set to true, scaling =
   * 90, dpi = 100, custom square paper, and custom margins.
   */
  test('SetStickySettings', function() {
    // Default state of the model.
    const stickySettingsDefault: {[key: string]: any} = {
      version: 2,
      recentDestinations: [],
      dpi: {},
      mediaSize: {},
      marginsType: 0, /* default */
      scaling: '100',
      scalingType: ScalingType.DEFAULT,
      scalingTypePdf: ScalingType.DEFAULT,
      isHeaderFooterEnabled: true,
      isCssBackgroundEnabled: false,
      isCollateEnabled: true,
      isDuplexEnabled: true,
      isDuplexShortEdge: false,
      isLandscapeEnabled: false,
      isColorEnabled: true,
      vendorOptions: {},
    };

    // Non-default state
    const stickySettingsChange: {[key: string]: any} = {
      version: 2,
      recentDestinations: [],
      dpi: {horizontal_dpi: 1000, vertical_dpi: 500},
      mediaSize: {width_microns: 43180, height_microns: 21590},
      marginsType: 2, /* none */
      scaling: '85',
      scalingType: ScalingType.CUSTOM,
      scalingTypePdf: ScalingType.FIT_TO_PAGE,
      isHeaderFooterEnabled: false,
      isCssBackgroundEnabled: true,
      isCollateEnabled: false,
      isDuplexEnabled: false,
      isDuplexShortEdge: true,
      isLandscapeEnabled: true,
      isColorEnabled: false,
      vendorOptions: {
        paperType: 1,
        printArea: 6,
      },
    };

    const settingsSet = ['version'];

    /**
     * @param setting The name of the setting to check.
     * @param field The name of the field in the serialized state
     *     corresponding to the setting.
     * @return Promise that resolves when the setting has been set,
     *     the saved string has been validated, and the setting has been
     *     reset to its default value.
     */
    const testStickySetting = function(
        setting: keyof Settings, field: string): Promise<void> {
      const promise = eventToPromise('sticky-setting-changed', model);
      model.setSetting(setting, stickySettingsChange[field]);
      settingsSet.push(field);
      return promise.then(
          /**
           * @param e Event containing the serialized settings
           * @return Promise that resolves when setting is reset.
           */
          function(e: CustomEvent<string>): Promise<void> {
            const settings = JSON.parse(e.detail);
            Object.keys(stickySettingsDefault).forEach(settingName => {
              const set = settingsSet.includes(settingName);
              assertEquals(set, settings[settingName] !== undefined);
              if (set) {
                const toCompare = settingName === field ? stickySettingsChange :
                                                          stickySettingsDefault;
                assertDeepEquals(toCompare[settingName], settings[settingName]);
              }
            });
            const restorePromise =
                eventToPromise('sticky-setting-changed', model);
            model.setSetting(setting, stickySettingsDefault[field]);
            return restorePromise;
          });
    };

    model.applyStickySettings();
    return testStickySetting('collate', 'isCollateEnabled')
        .then(() => testStickySetting('color', 'isColorEnabled'))
        .then(
            () => testStickySetting('cssBackground', 'isCssBackgroundEnabled'))
        .then(() => testStickySetting('dpi', 'dpi'))
        .then(() => testStickySetting('duplex', 'isDuplexEnabled'))
        .then(() => testStickySetting('duplexShortEdge', 'isDuplexShortEdge'))
        .then(() => testStickySetting('headerFooter', 'isHeaderFooterEnabled'))
        .then(() => testStickySetting('layout', 'isLandscapeEnabled'))
        .then(() => testStickySetting('margins', 'marginsType'))
        .then(() => testStickySetting('mediaSize', 'mediaSize'))
        .then(() => testStickySetting('scaling', 'scaling'))
        .then(() => testStickySetting('scalingType', 'scalingType'))
        .then(() => testStickySetting('scalingTypePdf', 'scalingTypePdf'))
        .then(() => testStickySetting('vendorItems', 'vendorOptions'));
  });

  /**
   * Tests that setSetting() won't change the value if there is already a
   * policy for that setting.
   */
  test('SetPolicySettings', function() {
    model.setSetting('headerFooter', false);
    assertFalse(model.settings.headerFooter.value as boolean);

    // Sets to true, but doesn't mark as controlled by a policy.
    model.setPolicySettings({headerFooter: {defaultMode: true}});
    model.setStickySettings(JSON.stringify({
      version: 2,
      headerFooter: false,
    }));
    model.applyStickySettings();
    assertTrue(model.settings.headerFooter.value as boolean);
    model.setSetting('headerFooter', false);
    assertFalse(model.settings.headerFooter.value as boolean);

    model.setPolicySettings({headerFooter: {allowedMode: true}});
    model.applyStickySettings();
    assertTrue(model.settings.headerFooter.value as boolean);

    model.setSetting('headerFooter', false);
    // The value didn't change after setSetting(), because the policy takes
    // priority.
    assertTrue(model.settings.headerFooter.value as boolean);
  });

  function toggleSettings(testDestination: Destination) {
    const settingsChange: {[key: string]: any} = {
      pages: [2],
      copies: 2,
      collate: false,
      layout: true,
      color: false,
      mediaSize: testDestination.capabilities!.printer.media_size!.option[1]!,
      mediaType: testDestination.capabilities!.printer.media_type!.option[1],
      margins: MarginsType.CUSTOM,
      customMargins: {
        marginTop: 100,
        marginRight: 200,
        marginBottom: 300,
        marginLeft: 400,
      },
      dpi: {
        horizontal_dpi: 100,
        vertical_dpi: 100,
      },
      scaling: '90',
      scalingType: ScalingType.CUSTOM,
      scalingTypePdf: ScalingType.CUSTOM,
      duplex: true,
      duplexShortEdge: true,
      cssBackground: true,
      selectionOnly: true,
      headerFooter: false,
      rasterize: true,
      vendorItems: {
        printArea: 6,
        paperType: 1,
      },
      ranges: [{from: 2, to: 2}],
    };

    // Update settings
    Object.keys(settingsChange).forEach(setting => {
      model.set(`settings.${setting}.value`, settingsChange[setting]);
    });
  }

  function initializeModel() {
    model.documentSettings = {
      allPagesHaveCustomSize: false,
      allPagesHaveCustomOrientation: false,
      hasSelection: true,
      isModifiable: true,
      isScalingDisabled: false,
      fitToPageScaling: 100,
      pageCount: 3,
      isFromArc: false,
      title: 'title',
    };
    model.pageSize = new Size(612, 792);

    // Update pages accordingly.
    model.set('settings.pages.value', [1, 2, 3]);

    // Initialize some settings that don't have defaults to the destination
    // defaults.
    model.set('settings.dpi.value', {horizontal_dpi: 200, vertical_dpi: 200});
    model.set('settings.vendorItems.value', {paperType: 0, printArea: 4});

    // Set rasterize available so that it can be tested.
    model.set('settings.rasterize.available', true);
  }

  /**
   * Tests that toggling each setting results in the expected change to the
   * print ticket.
   */
  test('GetPrintTicket', function() {
    const origin = DestinationOrigin.LOCAL;
    const testDestination = new Destination('FooDevice', origin, 'FooName');
    testDestination.capabilities =
        getCddTemplateWithAdvancedSettings(2, 'FooDevice').capabilities;

    loadTimeData.overrideValues({isBorderlessPrintingEnabled: true});
    initializeModel();
    model.destination = testDestination;
    const defaultTicket =
        model.createPrintTicket(testDestination, false, false);

    const expectedDefaultTicketObject: PrintTicket = {
      mediaSize: testDestination.capabilities!.printer.media_size!.option[0]!,
      mediaType: testDestination.capabilities!.printer.media_type!.option[0]!
                     .vendor_id,
      pageCount: 3,
      landscape: false,
      color: testDestination.getNativeColorModel(true),
      headerFooterEnabled: false,  // Only used in print preview
      marginsType: MarginsType.DEFAULT,
      duplex: DuplexMode.SIMPLEX,
      copies: 1,
      collate: true,
      shouldPrintBackgrounds: false,
      shouldPrintSelectionOnly: false,
      previewModifiable: true,
      printerType: PrinterType.LOCAL_PRINTER,
      rasterizePDF: false,
      scaleFactor: 100,
      scalingType: ScalingType.DEFAULT,
      pagesPerSheet: 1,
      dpiHorizontal: 200,
      dpiVertical: 200,
      dpiDefault: true,
      deviceName: 'FooDevice',
      pageWidth: 612,
      pageHeight: 792,
      showSystemDialog: false,
    };
    assertEquals(JSON.stringify(expectedDefaultTicketObject), defaultTicket);

    // Toggle all the values and create a new print ticket.
    toggleSettings(testDestination);
    const newTicket = model.createPrintTicket(testDestination, false, false);
    const expectedNewTicketObject: PrintTicket = {
      mediaSize: testDestination.capabilities!.printer.media_size!.option[1]!,
      borderless: false,
      mediaType: testDestination.capabilities!.printer.media_type!.option[1]!
                     .vendor_id,
      pageCount: 1,
      landscape: true,
      color: testDestination.getNativeColorModel(false),
      headerFooterEnabled: false,
      marginsType: MarginsType.CUSTOM,
      duplex: DuplexMode.SHORT_EDGE,
      copies: 2,
      collate: false,
      shouldPrintBackgrounds: true,
      shouldPrintSelectionOnly: false,  // Only for Print Preview.
      previewModifiable: true,
      printerType: PrinterType.LOCAL_PRINTER,
      rasterizePDF: true,
      scaleFactor: 90,
      scalingType: ScalingType.CUSTOM,
      pagesPerSheet: 1,
      dpiHorizontal: 100,
      dpiVertical: 100,
      dpiDefault: false,
      deviceName: 'FooDevice',
      pageWidth: 612,
      pageHeight: 792,
      showSystemDialog: false,
      marginsCustom: {
        marginTop: 100,
        marginRight: 200,
        marginBottom: 300,
        marginLeft: 400,
      },
    };

    assertEquals(JSON.stringify(expectedNewTicketObject), newTicket);
  });

  /**
   * Tests that toggling each setting results in the expected change to the
   * cloud job print ticket.
   */
  test('GetCloudPrintTicket', function() {
    initializeModel();

    // Create a test extension destination.
    const testDestination =
        new Destination('FooDevice', DestinationOrigin.EXTENSION, 'FooName');
    testDestination.capabilities =
        getCddTemplateWithAdvancedSettings(2, 'FooDevice').capabilities;
    model.destination = testDestination;

    const defaultTicket = model.createCloudJobTicket(testDestination);
    const expectedDefaultTicket = JSON.stringify({
      version: '1.0',
      print: {
        collate: {collate: true},
        color: {
          type: testDestination.getColor(true)!.type,
        },
        copies: {copies: 1},
        duplex: {type: 'NO_DUPLEX'},
        media_size: {
          width_microns: 215900,
          height_microns: 279400,
        },
        page_orientation: {type: 'PORTRAIT'},
        dpi: {
          horizontal_dpi: 200,
          vertical_dpi: 200,
        },
        vendor_ticket_item: [
          {id: 'printArea', value: 4},
          {id: 'paperType', value: 0},
        ],
      },
    });
    assertEquals(expectedDefaultTicket, defaultTicket);

    // Toggle all the values and create a new cloud job ticket.
    toggleSettings(testDestination);
    const newTicket = model.createCloudJobTicket(testDestination);
    const expectedNewTicket = JSON.stringify({
      version: '1.0',
      print: {
        collate: {collate: false},
        color: {
          type: testDestination.getColor(false)!.type,
        },
        copies: {copies: 2},
        duplex: {type: 'SHORT_EDGE'},
        media_size: {
          width_microns: 215900,
          height_microns: 215900,
        },
        page_orientation: {type: 'LANDSCAPE'},
        dpi: {
          horizontal_dpi: 100,
          vertical_dpi: 100,
        },
        vendor_ticket_item: [
          {id: 'printArea', value: 6},
          {id: 'paperType', value: 1},
        ],
      },
    });
    assertEquals(expectedNewTicket, newTicket);
  });

  test('RemoveUnsupportedDestinations', function() {
    const unsupportedPrivet =
        new Destination('PrivetDevice', DestinationOrigin.PRIVET, 'PrivetName');
    const unsupportedCloud =
        new Destination('CloudDevice', DestinationOrigin.COOKIES, 'CloudName');
    const supportedLocal =
        new Destination('FooDevice', DestinationOrigin.LOCAL, 'FooName');
    const stickySettings: {[key: string]: any} = {
      version: 2,
      recentDestinations: [
        makeRecentDestination(unsupportedPrivet),
        makeRecentDestination(unsupportedCloud),
        makeRecentDestination(supportedLocal),
      ],
    };

    initializeModel();
    model.setStickySettings(JSON.stringify(stickySettings));

    // Make sure recent destinations are filtered correctly.
    let recentDestinations =
        model.getSettingValue('recentDestinations') as RecentDestination[];
    assertEquals(1, recentDestinations.length);
    assertEquals('FooDevice', recentDestinations[0]!.id);

    // Setting this destination based on the recent printers is done by the
    // destination store in the production code.
    model.destination = supportedLocal;
    model.applyStickySettings();
    model.applyPoliciesOnDestinationUpdate();

    // Make sure nothing changed.
    recentDestinations =
        model.getSettingValue('recentDestinations') as RecentDestination[];
    assertEquals(1, recentDestinations.length);
    assertEquals('FooDevice', recentDestinations[0]!.id);
  });

  test('ChangeDestination', function() {
    const testDestination =
        new Destination('FooDevice', DestinationOrigin.LOCAL, 'FooName');
    testDestination.capabilities =
        getCddTemplateWithAdvancedSettings(2, 'FooDevice').capabilities;
    // Make black and white printing the default.
    testDestination.capabilities!.printer.color = {
      option: [
        {type: 'STANDARD_COLOR'},
        {type: 'STANDARD_MONOCHROME', is_default: true},
      ] as ColorOption[],
    };

    const testDestination2 =
        new Destination('BarDevice', DestinationOrigin.LOCAL, 'BarName');
    testDestination2.capabilities =
        Object.assign({}, testDestination.capabilities);

    // Initialize
    initializeModel();
    model.destination = testDestination;
    model.applyStickySettings();

    // Confirm some defaults.
    assertEquals(false, model.getSettingValue('color'));
    assertEquals('NA_LETTER', model.getSettingValue('mediaSize').name);
    assertEquals(200, model.getSettingValue('dpi').horizontal_dpi);
    assertEquals(false, model.getSettingValue('duplex'));

    // Toggle some printer specified settings.
    model.setSetting('duplex', true);
    model.setSetting(
        'mediaSize',
        testDestination.capabilities!.printer.media_size!.option[1]!);
    model.setSetting('color', true);
    model.setSetting(
        'dpi', testDestination.capabilities!.printer.dpi!.option[1]!);

    // Confirm toggles.
    assertEquals(true, model.getSettingValue('color'));
    assertEquals('CUSTOM', model.getSettingValue('mediaSize').name);
    assertEquals(100, model.getSettingValue('dpi').horizontal_dpi);
    assertEquals(true, model.getSettingValue('duplex'));

    // Set to a new destination with the same capabilities. Confirm that
    // everything stays the same.
    const oldSettings = JSON.stringify(model.settings);
    model.destination = testDestination2;
    const newSettings = JSON.stringify(model.settings);

    // Should be the same (same printer capabilities).
    assertEquals(oldSettings, newSettings);

    // Create a printer with different capabilities.
    const testDestination3 =
        new Destination('Device1', DestinationOrigin.LOCAL, 'One');
    testDestination3.capabilities =
        Object.assign({}, testDestination.capabilities);
    testDestination3.capabilities.printer.media_size = {
      option: [
        {
          name: 'ISO_A4',
          width_microns: 210000,
          height_microns: 297000,
          custom_display_name: 'A4',
          is_default: true,
        },
      ],
    };
    testDestination3.capabilities.printer.color = {
      option: [
        {type: 'STANDARD_MONOCHROME', is_default: true},
      ] as ColorOption[],
    };
    testDestination3.capabilities.printer.duplex = {
      option: [
        {type: 'NO_DUPLEX', is_default: true},
      ] as DuplexOption[],
    };
    testDestination3.capabilities.printer.dpi = {
      option: [
        {horizontal_dpi: 400, vertical_dpi: 400, is_default: true},
        {horizontal_dpi: 800, vertical_dpi: 800},
      ] as DpiOption[],
    };

    model.destination = testDestination3;
    flush();

    // Verify things changed.
    const updatedSettings = JSON.stringify(model.settings);
    assertNotEquals(oldSettings, updatedSettings);
    assertEquals(false, model.getSettingValue('color'));
    assertEquals('ISO_A4', model.getSettingValue('mediaSize').name);
    assertEquals(400, model.getSettingValue('dpi').horizontal_dpi);
    assertEquals(false, model.getSettingValue('duplex'));
  });

  /**
   * Tests the behaviour of the CDD attribute `reset_to_default`, specifically
   * that when a setting has a default value in CDD and the user selects another
   * value in the UI:
   * - if `reset_to_default`=true, the value of the setting will always be reset
   * to the CDD default.
   * - if `reset_to_default`=false, the value of the setting will always be read
   * from the sticky settings.
   */
  test('CddResetToDefault', function() {
    const cddColorEnabled = true;
    const stickyColorEnabled = false;
    const cddDuplexEnabled = false;
    const stickyDuplexEnabled = true;
    const cddDpi = 200;
    const stickyDpi = 100;
    const cddMediaSizeDisplayName = 'CDD_NAME';
    const stickyMediaSizeDisplayName = 'STICKY_NAME';

    /**
     * Returns the CDD description of a destination with default values
     * specified for color, dpi, duplex and media size.
     * @param resetToDefault Whether the settings should
     * always reset to their default value or not.
     */
    const getTestCapabilities = (resetToDefault: boolean) => {
      return {
        version: '1.0',
        printer: {
          color: {
            option: [
              {type: 'STANDARD_COLOR', is_default: true},
              {type: 'STANDARD_MONOCHROME'},
            ],
            reset_to_default: resetToDefault,
          },
          duplex: {
            option: [
              {type: 'NO_DUPLEX', is_default: true},
              {type: 'LONG_EDGE'},
              {type: 'SHORT_EDGE'},
            ],
            reset_to_default: resetToDefault,
          },
          dpi: {
            option: [
              {
                horizontal_dpi: cddDpi,
                vertical_dpi: cddDpi,
                is_default: true,
              },
              {horizontal_dpi: stickyDpi, vertical_dpi: stickyDpi},
            ],
            reset_to_default: resetToDefault,
          },
          media_size: {
            option: [
              {
                name: 'NA_LETTER',
                width_microns: 215900,
                height_microns: 279400,
                is_default: true,
                custom_display_name: cddMediaSizeDisplayName,
              },
              {
                name: 'CUSTOM',
                width_microns: 215900,
                height_microns: 215900,
                custom_display_name: stickyMediaSizeDisplayName,
              },
            ],
            reset_to_default: resetToDefault,
          },
        },
      };
    };
    // Sticky settings that contain different values from the default values
    // returned by `getTestCapabilities`.
    const stickySettings = {
      version: 2,
      isColorEnabled: stickyColorEnabled,
      isDuplexEnabled: stickyDuplexEnabled,
      dpi: {horizontal_dpi: stickyDpi, vertical_dpi: stickyDpi},
      mediaSize: {
        name: 'CUSTOM',
        width_microns: 215900,
        height_microns: 215900,
        custom_display_name: stickyMediaSizeDisplayName,
      },
    };

    const testDestination =
        new Destination('FooDevice', DestinationOrigin.EXTENSION, 'FooName');
    testDestination.capabilities =
        getTestCapabilities(/*resetToDefault=*/ true);
    initializeModel();
    model.destination = testDestination;
    model.setStickySettings(JSON.stringify(stickySettings));
    model.applyStickySettings();
    assertEquals(model.settings.color.value, cddColorEnabled);
    assertEquals(model.settings.duplex.value, cddDuplexEnabled);
    assertEquals(model.settings.dpi.value.horizontal_dpi, cddDpi);
    assertEquals(
        model.settings.mediaSize.value.custom_display_name,
        cddMediaSizeDisplayName);

    testDestination.capabilities =
        getTestCapabilities(/*resetToDefault=*/ false);
    model.destination = testDestination;
    model.setStickySettings(JSON.stringify(stickySettings));
    model.applyStickySettings();
    assertEquals(model.settings.color.value, stickyColorEnabled);
    assertEquals(model.settings.duplex.value, stickyDuplexEnabled);
    assertEquals(model.settings.dpi.value.horizontal_dpi, stickyDpi);
    assertEquals(
        model.settings.mediaSize.value.custom_display_name,
        stickyMediaSizeDisplayName);

    const testDestination2 =
        new Destination('FooDevice2', DestinationOrigin.EXTENSION, 'FooName2');
    testDestination2.capabilities =
        getTestCapabilities(/*resetToDefault=*/ true);
    // Remove the `is_default` attribute from all the settings.
    delete testDestination2.capabilities.printer.color!.option[0]!.is_default;
    delete testDestination2.capabilities.printer.duplex!.option[0]!.is_default;
    delete testDestination2.capabilities.printer.media_size!.option[0]!
        .is_default;
    delete testDestination2.capabilities.printer.dpi!.option[0]!.is_default;

    model.destination = testDestination2;

    // Even if `reset_to_default` is true for all options, the model settings
    // should have values from the sticky settings because the CDD doesn't
    // specify default values to reset to.
    model.setStickySettings(JSON.stringify(stickySettings));
    model.applyStickySettings();
    assertEquals(model.settings.color.value, stickyColorEnabled);
    assertEquals(model.settings.duplex.value, stickyDuplexEnabled);
    assertEquals(model.settings.dpi.value.horizontal_dpi, stickyDpi);
    assertEquals(
        model.settings.mediaSize.value.custom_display_name,
        stickyMediaSizeDisplayName);
  });

  /**
   * Tests that setStickySettings() stores custom margins as integers.
   */
  test('CustomMarginsAreInts', function() {
    model.setStickySettings(JSON.stringify({
      version: 2,
      customMargins: {
        marginTop: 100.5,
        marginRight: 200,
        marginBottom: 333.333333,
        marginLeft: 400,
      },
      marginsType: MarginsType.CUSTOM,
    }));
    model.applyStickySettings();
    assertEquals(model.settings.margins.value, MarginsType.CUSTOM);
    assertTrue('marginTop' in model.settings.customMargins.value);
    assertTrue('marginRight' in model.settings.customMargins.value);
    assertTrue('marginBottom' in model.settings.customMargins.value);
    assertTrue('marginLeft' in model.settings.customMargins.value);
    assertEquals(model.settings.customMargins.value.marginTop, 101);
    assertEquals(model.settings.customMargins.value.marginRight, 200);
    assertEquals(model.settings.customMargins.value.marginBottom, 333);
    assertEquals(model.settings.customMargins.value.marginLeft, 400);
  });

  /**
   * Tests that if setStickySettings() stored the margins type as custom, but
   * have no customMargins, then fall back to the default margins type.
   */
  test('CustomMarginsAreNotEmpty', function() {
    model.setStickySettings(JSON.stringify({
      version: 2,
      marginsType: MarginsType.CUSTOM,
    }));
    model.applyStickySettings();
    assertMarginsSettingsResetToDefault(model.settings);
  });

  /**
   * Tests that if setStickySettings() stored negative custom margins, then fall
   * back to the default margins type.
   */
  test('CustomMarginsAreNotNegative', function() {
    model.setStickySettings(JSON.stringify({
      version: 2,
      customMargins: {
        marginTop: 100,
        marginRight: 200,
        marginBottom: -333,
        marginLeft: 400,
      },
      marginsType: MarginsType.CUSTOM,
    }));
    model.applyStickySettings();
    assertMarginsSettingsResetToDefault(model.settings);
  });

  /**
   * Tests that if setStickySettings() stored custom margins as strings, then
   * fall back to the default margins type.
   */
  test('CustomMarginsAreNotStrings', function() {
    model.setStickySettings(JSON.stringify({
      version: 2,
      customMargins: {
        marginTop: 100,
        marginRight: 200,
        marginBottom: 333,
        marginLeft: 'bad',
      },
      marginsType: MarginsType.CUSTOM,
    }));
    model.applyStickySettings();
    assertMarginsSettingsResetToDefault(model.settings);
  });
});
