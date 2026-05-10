// include: shell.js
// The Module object: Our interface to the outside world. We import
// and export values on it. There are various ways Module can be used:
// 1. Not defined. We create it here
// 2. A function parameter, function(moduleArg) => Promise<Module>
// 3. pre-run appended it, var Module = {}; ..generated code..
// 4. External script tag defines var Module.
// We need to check if Module already exists (e.g. case 3 above).
// Substitution will be replaced with actual code on later stage of the build,
// this way Closure Compiler will not mangle it (e.g. case 4. above).
// Note that if you want to run closure, and also to use Module
// after the generated code, you will need to define   var Module = {};
// before the code. Then that object will be used in the code, and you
// can continue to use Module afterwards as well.
var Module = typeof Module != 'undefined' ? Module : {};

// Determine the runtime environment we are in. You can customize this by
// setting the ENVIRONMENT setting at compile time (see settings.js).

// Attempt to auto-detect the environment
var ENVIRONMENT_IS_WEB = typeof window == 'object';
var ENVIRONMENT_IS_WORKER = typeof WorkerGlobalScope != 'undefined';
// N.b. Electron.js environment is simultaneously a NODE-environment, but
// also a web environment.
var ENVIRONMENT_IS_NODE = typeof process == 'object' && process.versions?.node && process.type != 'renderer';
var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;

if (ENVIRONMENT_IS_NODE) {

}

// --pre-jses are emitted after the Module integration code, so that they can
// refer to Module (if they choose; they can also define Module)
// include: C:\Users\bogda_\AppData\Local\Temp\tmptd0r_bya.js

  Module['expectedDataFileDownloads'] ??= 0;
  Module['expectedDataFileDownloads']++;
  (() => {
    // Do not attempt to redownload the virtual filesystem data when in a pthread or a Wasm Worker context.
    var isPthread = typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD;
    var isWasmWorker = typeof ENVIRONMENT_IS_WASM_WORKER != 'undefined' && ENVIRONMENT_IS_WASM_WORKER;
    if (isPthread || isWasmWorker) return;
    var isNode = typeof process === 'object' && typeof process.versions === 'object' && typeof process.versions.node === 'string';
    function loadPackage(metadata) {

      var PACKAGE_PATH = '';
      if (typeof window === 'object') {
        PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.substring(0, window.location.pathname.lastIndexOf('/')) + '/');
      } else if (typeof process === 'undefined' && typeof location !== 'undefined') {
        // web worker
        PACKAGE_PATH = encodeURIComponent(location.pathname.substring(0, location.pathname.lastIndexOf('/')) + '/');
      }
      var PACKAGE_NAME = 'ShooterGame/bin/main.data';
      var REMOTE_PACKAGE_BASE = 'main.data';
      var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
var REMOTE_PACKAGE_SIZE = metadata['remote_package_size'];

      function fetchRemotePackage(packageName, packageSize, callback, errback) {
        if (isNode) {
          require('fs').readFile(packageName, (err, contents) => {
            if (err) {
              errback(err);
            } else {
              callback(contents.buffer);
            }
          });
          return;
        }
        Module['dataFileDownloads'] ??= {};
        fetch(packageName)
          .catch((cause) => Promise.reject(new Error(`Network Error: ${packageName}`, {cause}))) // If fetch fails, rewrite the error to include the failing URL & the cause.
          .then((response) => {
            if (!response.ok) {
              return Promise.reject(new Error(`${response.status}: ${response.url}`));
            }

            if (!response.body && response.arrayBuffer) { // If we're using the polyfill, readers won't be available...
              return response.arrayBuffer().then(callback);
            }

            const reader = response.body.getReader();
            const iterate = () => reader.read().then(handleChunk).catch((cause) => {
              return Promise.reject(new Error(`Unexpected error while handling : ${response.url} ${cause}`, {cause}));
            });

            const chunks = [];
            const headers = response.headers;
            const total = Number(headers.get('Content-Length') ?? packageSize);
            let loaded = 0;

            const handleChunk = ({done, value}) => {
              if (!done) {
                chunks.push(value);
                loaded += value.length;
                Module['dataFileDownloads'][packageName] = {loaded, total};

                let totalLoaded = 0;
                let totalSize = 0;

                for (const download of Object.values(Module['dataFileDownloads'])) {
                  totalLoaded += download.loaded;
                  totalSize += download.total;
                }

                Module['setStatus']?.(`Downloading data... (${totalLoaded}/${totalSize})`);
                return iterate();
              } else {
                const packageData = new Uint8Array(chunks.map((c) => c.length).reduce((a, b) => a + b, 0));
                let offset = 0;
                for (const chunk of chunks) {
                  packageData.set(chunk, offset);
                  offset += chunk.length;
                }
                callback(packageData.buffer);
              }
            };

            Module['setStatus']?.('Downloading data...');
            return iterate();
          });
      };

      function handleError(error) {
        console.error('package error:', error);
      };

      var fetchedCallback = null;
      var fetched = Module['getPreloadedPackage'] ? Module['getPreloadedPackage'](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE) : null;

      if (!fetched) fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE, (data) => {
        if (fetchedCallback) {
          fetchedCallback(data);
          fetchedCallback = null;
        } else {
          fetched = data;
        }
      }, handleError);

    function runWithFS(Module) {

      function assert(check, msg) {
        if (!check) throw msg + new Error().stack;
      }
Module['FS_createPath']("/", "GameData", true, true);
Module['FS_createPath']("/GameData", "PSOs", true, true);
Module['FS_createPath']("/GameData", "animations", true, true);
Module['FS_createPath']("/GameData/animations", "npc", true, true);
Module['FS_createPath']("/GameData/animations", "player", true, true);
Module['FS_createPath']("/GameData/animations/player", "body", true, true);
Module['FS_createPath']("/GameData", "behaviourTrees", true, true);
Module['FS_createPath']("/GameData", "env", true, true);
Module['FS_createPath']("/GameData/env", "example", true, true);
Module['FS_createPath']("/GameData", "fonts", true, true);
Module['FS_createPath']("/GameData", "scripts", true, true);
Module['FS_createPath']("/GameData", "shaders", true, true);
Module['FS_createPath']("/GameData/shaders", "compiled", true, true);
Module['FS_createPath']("/GameData/shaders/compiled", "linux", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux", "gl", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/gl", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/gl", "debugDraw", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/gl", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/gl", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/gl", "ui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux", "spirv", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/spirv", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/spirv", "debugDraw", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/spirv", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/spirv", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/spirv", "ui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled", "web", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web", "gles", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/gles", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/gles", "debugDraw", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/gles", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/gles", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/gles", "ui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web", "spirv", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/spirv", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/spirv", "debugDraw", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/spirv", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/spirv", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/spirv", "ui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled", "windows", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows", "dx11", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx11", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx11", "debugDraw", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx11", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx11", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx11", "ui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows", "dx12", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx12", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx12", "debugDraw", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx12", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx12", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx12", "ui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows", "gl", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/gl", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/gl", "debugDraw", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/gl", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/gl", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/gl", "ui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows", "spirv", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/spirv", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/spirv", "debugDraw", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/spirv", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/spirv", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/spirv", "ui", true, true);
Module['FS_createPath']("/GameData/shaders", "source", true, true);
Module['FS_createPath']("/GameData/shaders/source", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/source", "debugDraw", true, true);
Module['FS_createPath']("/GameData/shaders/source", "game", true, true);
Module['FS_createPath']("/GameData/shaders/source", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/source", "ui", true, true);
Module['FS_createPath']("/GameData", "sounds", true, true);
Module['FS_createPath']("/GameData/sounds", "banks", true, true);
Module['FS_createPath']("/GameData/sounds/banks", "Desktop", true, true);
Module['FS_createPath']("/GameData/sounds", "dog", true, true);
Module['FS_createPath']("/GameData/sounds", "weapons", true, true);
Module['FS_createPath']("/GameData/sounds/weapons", "shotgun", true, true);
Module['FS_createPath']("/GameData", "tables", true, true);
Module['FS_createPath']("/GameData/tables", "items", true, true);
Module['FS_createPath']("/GameData/tables", "npc", true, true);
Module['FS_createPath']("/GameData", "textures", true, true);
Module['FS_createPath']("/GameData/textures", "FPSTextures", true, true);
Module['FS_createPath']("/GameData/textures", "FPSTextures_HD", true, true);
Module['FS_createPath']("/GameData/textures", "Ground", true, true);
Module['FS_createPath']("/GameData/textures", "building_1", true, true);
Module['FS_createPath']("/GameData/textures", "common", true, true);
Module['FS_createPath']("/GameData/textures", "delvenPack", true, true);
Module['FS_createPath']("/GameData/textures", "generic", true, true);
Module['FS_createPath']("/GameData/textures", "lq_conc", true, true);
Module['FS_createPath']("/GameData/textures", "lq_dev", true, true);
Module['FS_createPath']("/GameData/textures", "lq_flesh", true, true);
Module['FS_createPath']("/GameData/textures", "lq_greek", true, true);
Module['FS_createPath']("/GameData/textures", "lq_health_ammo", true, true);
Module['FS_createPath']("/GameData/textures", "lq_legacy", true, true);
Module['FS_createPath']("/GameData/textures", "lq_liquidsky", true, true);
Module['FS_createPath']("/GameData/textures", "lq_mayan", true, true);
Module['FS_createPath']("/GameData/textures", "lq_medieval", true, true);
Module['FS_createPath']("/GameData/textures", "lq_metal", true, true);
Module['FS_createPath']("/GameData/textures", "lq_palette", true, true);
Module['FS_createPath']("/GameData/textures", "lq_props", true, true);
Module['FS_createPath']("/GameData/textures", "lq_tech", true, true);
Module['FS_createPath']("/GameData/textures", "lq_terra", true, true);
Module['FS_createPath']("/GameData/textures", "lq_utility", true, true);
Module['FS_createPath']("/GameData/textures", "lq_wood", true, true);
Module['FS_createPath']("/GameData/textures", "metal", true, true);
Module['FS_createPath']("/GameData/textures", "noise", true, true);
Module['FS_createPath']("/GameData/textures", "particles", true, true);
Module['FS_createPath']("/GameData/textures", "pp", true, true);
Module['FS_createPath']("/GameData/textures", "skies", true, true);
Module['FS_createPath']("/GameData/textures", "tormentPack", true, true);
Module['FS_createPath']("/GameData/textures", "ui", true, true);
Module['FS_createPath']("/GameData/textures/ui", "debuffs", true, true);
Module['FS_createPath']("/GameData/textures", "wall", true, true);
Module['FS_createPath']("/GameData/textures", "water", true, true);
Module['FS_createPath']("/GameData/textures", "wood", true, true);
Module['FS_createPath']("/GameData", "ui", true, true);
Module['FS_createPath']("/GameData", "videos", true, true);

      /** @constructor */
      function DataRequest(start, end, audio) {
        this.start = start;
        this.end = end;
        this.audio = audio;
      }
      DataRequest.prototype = {
        requests: {},
        open: function(mode, name) {
          this.name = name;
          this.requests[name] = this;
          Module['addRunDependency'](`fp ${this.name}`);
        },
        send: function() {},
        onload: function() {
          var byteArray = this.byteArray.subarray(this.start, this.end);
          this.finish(byteArray);
        },
        finish: function(byteArray) {
          var that = this;
          // canOwn this data in the filesystem, it is a slide into the heap that will never change
          Module['FS_createDataFile'](this.name, null, byteArray, true, true, true);
          Module['removeRunDependency'](`fp ${that.name}`);
          this.requests[this.name] = null;
        }
      };

      var files = metadata['files'];
      for (var i = 0; i < files.length; ++i) {
        new DataRequest(files[i]['start'], files[i]['end'], files[i]['audio'] || 0).open('GET', files[i]['filename']);
      }

      function processPackageData(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file failed.');
        assert(arrayBuffer.constructor.name === ArrayBuffer.name, 'bad input to processPackageData');
        var byteArray = new Uint8Array(arrayBuffer);
        var curr;
        // Reuse the bytearray from the XHR as the source for file reads.
          DataRequest.prototype.byteArray = byteArray;
          var files = metadata['files'];
          for (var i = 0; i < files.length; ++i) {
            DataRequest.prototype.requests[files[i].filename].onload();
          }          Module['removeRunDependency']('datafile_ShooterGame/bin/main.data');

      };
      Module['addRunDependency']('datafile_ShooterGame/bin/main.data');

      Module['preloadResults'] ??= {};

      Module['preloadResults'][PACKAGE_NAME] = {fromCache: false};
      if (fetched) {
        processPackageData(fetched);
        fetched = null;
      } else {
        fetchedCallback = processPackageData;
      }

    }
    if (Module['calledRun']) {
      runWithFS(Module);
    } else {
      (Module['preRun'] ??= []).push(runWithFS); // FS is not initialized yet, wait for it
    }

    }
    loadPackage({"files": [{"filename": "/GameData/PSOs/pso_cache.json", "start": 0, "end": 6054}, {"filename": "/GameData/animations/npc/WrithingInPain.fbx", "start": 6054, "end": 1690422}, {"filename": "/GameData/animations/npc/caution_idle.glb", "start": 1690422, "end": 3555018}, {"filename": "/GameData/animations/npc/idle.glb", "start": 3555018, "end": 5367498}, {"filename": "/GameData/animations/npc/inPain.glb", "start": 5367498, "end": 7288170}, {"filename": "/GameData/animations/npc/run.glb", "start": 7288170, "end": 9116530}, {"filename": "/GameData/animations/npc/standUp.glb", "start": 9116530, "end": 11455810}, {"filename": "/GameData/animations/npc/walk.glb", "start": 11455810, "end": 12196438}, {"filename": "/GameData/animations/player/body/idle.glb", "start": 12196438, "end": 14008918}, {"filename": "/GameData/animations/player/body/run_f.glb", "start": 14008918, "end": 15837278}, {"filename": "/GameData/arms.glb", "start": 15837278, "end": 16017042}, {"filename": "/GameData/behaviourTrees/general.bt", "start": 16017042, "end": 16103534}, {"filename": "/GameData/behaviourTrees/test", "start": 16103534, "end": 16104632}, {"filename": "/GameData/behaviourTrees/test2", "start": 16104632, "end": 16110540}, {"filename": "/GameData/behaviourTrees/test3", "start": 16110540, "end": 16112449}, {"filename": "/GameData/cat.png", "start": 16112449, "end": 16361887}, {"filename": "/GameData/cube.mtl", "start": 16361887, "end": 16361940}, {"filename": "/GameData/env/example/skybox2_overcast_cube_bk.png", "start": 16361940, "end": 16855410}, {"filename": "/GameData/env/example/skybox2_overcast_cube_dn.png", "start": 16855410, "end": 17637078}, {"filename": "/GameData/env/example/skybox2_overcast_cube_ft.png", "start": 17637078, "end": 18051863}, {"filename": "/GameData/env/example/skybox2_overcast_cube_lf.png", "start": 18051863, "end": 18488474}, {"filename": "/GameData/env/example/skybox2_overcast_cube_rt.png", "start": 18488474, "end": 18950942}, {"filename": "/GameData/env/example/skybox2_overcast_cube_up.png", "start": 18950942, "end": 19273209}, {"filename": "/GameData/env/notes.txt", "start": 19273209, "end": 19273297}, {"filename": "/GameData/env/skybox1_night_cube.png", "start": 19273297, "end": 21362311}, {"filename": "/GameData/env/skybox2_overcast_cube_bk.png", "start": 21362311, "end": 22042413}, {"filename": "/GameData/env/skybox2_overcast_cube_dn.png", "start": 22042413, "end": 23029007}, {"filename": "/GameData/env/skybox2_overcast_cube_dn_.png", "start": 23029007, "end": 24014864}, {"filename": "/GameData/env/skybox2_overcast_cube_ft.png", "start": 24014864, "end": 24620736}, {"filename": "/GameData/env/skybox2_overcast_cube_lf.png", "start": 24620736, "end": 25268575}, {"filename": "/GameData/env/skybox2_overcast_cube_rt.png", "start": 25268575, "end": 25918698}, {"filename": "/GameData/env/skybox2_overcast_cube_up.png", "start": 25918698, "end": 26413308}, {"filename": "/GameData/env/skybox2_overcast_cube_up_.png", "start": 26413308, "end": 26907851}, {"filename": "/GameData/env/skybox2_overcast_night_cube.png", "start": 26907851, "end": 28232094}, {"filename": "/GameData/env/skybox2_overcast_night_cube_bk.png", "start": 28232094, "end": 29390478}, {"filename": "/GameData/env/skybox2_overcast_night_cube_dn.png", "start": 29390478, "end": 30897955}, {"filename": "/GameData/env/skybox2_overcast_night_cube_ft.png", "start": 30897955, "end": 31971661}, {"filename": "/GameData/env/skybox2_overcast_night_cube_lf.png", "start": 31971661, "end": 33062855}, {"filename": "/GameData/env/skybox2_overcast_night_cube_rt.png", "start": 33062855, "end": 34235864}, {"filename": "/GameData/env/skybox2_overcast_night_cube_up.png", "start": 34235864, "end": 35051422}, {"filename": "/GameData/fonts/Font Awesome 6 Free-Regular-400.ttf", "start": 35051422, "end": 35127162}, {"filename": "/GameData/fonts/Kingthings_Calligraphica_2.ttf", "start": 35127162, "end": 35156966}, {"filename": "/GameData/fonts/fa-regular-400.ttf", "start": 35156966, "end": 35225030}, {"filename": "/GameData/maps.zip", "start": 35225030, "end": 40940648}, {"filename": "/GameData/models.zip", "start": 40940648, "end": 98479499}, {"filename": "/GameData/scripts/common.shader", "start": 98479499, "end": 98480312}, {"filename": "/GameData/scripts/skies.shader", "start": 98480312, "end": 98481978}, {"filename": "/GameData/shaders/compiled/linux/gl/bsp/fs_bsp.bin", "start": 98481978, "end": 98487429}, {"filename": "/GameData/shaders/compiled/linux/gl/bsp/fs_bsp_cube.bin", "start": 98487429, "end": 98487854}, {"filename": "/GameData/shaders/compiled/linux/gl/bsp/fs_bsp_empty.bin", "start": 98487854, "end": 98487934}, {"filename": "/GameData/shaders/compiled/linux/gl/bsp/vs_bsp.bin", "start": 98487934, "end": 98490229}, {"filename": "/GameData/shaders/compiled/linux/gl/debugDraw/fs_debugdraw.bin", "start": 98490229, "end": 98490311}, {"filename": "/GameData/shaders/compiled/linux/gl/debugDraw/vs_debugdraw.bin", "start": 98490311, "end": 98490596}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_bsp.bin", "start": 98490596, "end": 98496047}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_bsp_cube.bin", "start": 98496047, "end": 98496472}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_bsp_empty.bin", "start": 98496472, "end": 98496552}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_copy.bin", "start": 98496552, "end": 98496722}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_copy_depth.bin", "start": 98496722, "end": 98496891}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_customId.bin", "start": 98496891, "end": 98497674}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_default.bin", "start": 98497674, "end": 98504781}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_default_simple.bin", "start": 98504781, "end": 98508576}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_empty.bin", "start": 98508576, "end": 98508656}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_fxaa_simple.bin", "start": 98508656, "end": 98512114}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_mask.bin", "start": 98512114, "end": 98512375}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_motionBlur.bin", "start": 98512375, "end": 98514399}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_motionBlur_apply.bin", "start": 98514399, "end": 98514811}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_postprocessing.bin", "start": 98514811, "end": 98521628}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_resolve_depth_msaa.bin", "start": 98521628, "end": 98522204}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_solidRed.bin", "start": 98522204, "end": 98522284}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_ui_flatcolor.bin", "start": 98522284, "end": 98522387}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_ui_textured.bin", "start": 98522387, "end": 98522727}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_ui_textured_point.bin", "start": 98522727, "end": 98523192}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_unlit.bin", "start": 98523192, "end": 98523604}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_unlit_rect.bin", "start": 98523604, "end": 98524102}, {"filename": "/GameData/shaders/compiled/linux/gl/game/fs_debuff.bin", "start": 98524102, "end": 98526579}, {"filename": "/GameData/shaders/compiled/linux/gl/game/fs_inventory_menu_sphere.bin", "start": 98526579, "end": 98526722}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_blendmask.bin", "start": 98526722, "end": 98526974}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_blur.bin", "start": 98526974, "end": 98530099}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_color.bin", "start": 98530099, "end": 98530181}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_colormatrix.bin", "start": 98530181, "end": 98530810}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_creation.bin", "start": 98530810, "end": 98530890}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_dropshadow.bin", "start": 98530890, "end": 98531196}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_gradient.bin", "start": 98531196, "end": 98535242}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_passthrough.bin", "start": 98535242, "end": 98535403}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_texture.bin", "start": 98535403, "end": 98535595}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/vs_rmlui.bin", "start": 98535595, "end": 98535999}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/vs_rmlui_passthrough.bin", "start": 98535999, "end": 98536425}, {"filename": "/GameData/shaders/compiled/linux/gl/ui/fs_progressBar.bin", "start": 98536425, "end": 98537026}, {"filename": "/GameData/shaders/compiled/linux/gl/vs_bsp.bin", "start": 98537026, "end": 98539321}, {"filename": "/GameData/shaders/compiled/linux/gl/vs_default.bin", "start": 98539321, "end": 98543987}, {"filename": "/GameData/shaders/compiled/linux/gl/vs_fullscreen.bin", "start": 98543987, "end": 98544222}, {"filename": "/GameData/shaders/compiled/linux/gl/vs_instanced_billboard.bin", "start": 98544222, "end": 98545722}, {"filename": "/GameData/shaders/compiled/linux/gl/vs_ui.bin", "start": 98545722, "end": 98546076}, {"filename": "/GameData/shaders/compiled/linux/spirv/bsp/fs_bsp.bin", "start": 98546076, "end": 98554955}, {"filename": "/GameData/shaders/compiled/linux/spirv/bsp/fs_bsp_cube.bin", "start": 98554955, "end": 98556532}, {"filename": "/GameData/shaders/compiled/linux/spirv/bsp/fs_bsp_empty.bin", "start": 98556532, "end": 98556894}, {"filename": "/GameData/shaders/compiled/linux/spirv/bsp/vs_bsp.bin", "start": 98556894, "end": 98560608}, {"filename": "/GameData/shaders/compiled/linux/spirv/debugDraw/fs_debugdraw.bin", "start": 98560608, "end": 98561014}, {"filename": "/GameData/shaders/compiled/linux/spirv/debugDraw/vs_debugdraw.bin", "start": 98561014, "end": 98562074}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_bsp.bin", "start": 98562074, "end": 98570953}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_bsp_cube.bin", "start": 98570953, "end": 98572530}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_bsp_empty.bin", "start": 98572530, "end": 98572892}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_copy.bin", "start": 98572892, "end": 98573738}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_copy_depth.bin", "start": 98573738, "end": 98574607}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_customId.bin", "start": 98574607, "end": 98576472}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_default.bin", "start": 98576472, "end": 98585009}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_default_simple.bin", "start": 98585009, "end": 98591102}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_empty.bin", "start": 98591102, "end": 98591464}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_fxaa_simple.bin", "start": 98591464, "end": 98595131}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_mask.bin", "start": 98595131, "end": 98596017}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_motionBlur.bin", "start": 98596017, "end": 98600235}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_motionBlur_apply.bin", "start": 98600235, "end": 98601595}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_postprocessing.bin", "start": 98601595, "end": 98610647}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_resolve_depth_msaa.bin", "start": 98610647, "end": 98612174}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_solidRed.bin", "start": 98612174, "end": 98612552}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_ui_flatcolor.bin", "start": 98612552, "end": 98613136}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_ui_textured.bin", "start": 98613136, "end": 98614352}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_ui_textured_point.bin", "start": 98614352, "end": 98615896}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_unlit.bin", "start": 98615896, "end": 98617163}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_unlit_rect.bin", "start": 98617163, "end": 98618640}, {"filename": "/GameData/shaders/compiled/linux/spirv/game/fs_debuff.bin", "start": 98618640, "end": 98621605}, {"filename": "/GameData/shaders/compiled/linux/spirv/game/fs_inventory_menu_sphere.bin", "start": 98621605, "end": 98621999}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_blendmask.bin", "start": 98621999, "end": 98623039}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_blur.bin", "start": 98623039, "end": 98625667}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_color.bin", "start": 98625667, "end": 98626073}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_colormatrix.bin", "start": 98626073, "end": 98628043}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_creation.bin", "start": 98628043, "end": 98628405}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_dropshadow.bin", "start": 98628405, "end": 98629656}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_gradient.bin", "start": 98629656, "end": 98632884}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_passthrough.bin", "start": 98632884, "end": 98633639}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_texture.bin", "start": 98633639, "end": 98634502}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/vs_rmlui.bin", "start": 98634502, "end": 98635816}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/vs_rmlui_passthrough.bin", "start": 98635816, "end": 98637218}, {"filename": "/GameData/shaders/compiled/linux/spirv/ui/fs_progressBar.bin", "start": 98637218, "end": 98639127}, {"filename": "/GameData/shaders/compiled/linux/spirv/vs_bsp.bin", "start": 98639127, "end": 98642841}, {"filename": "/GameData/shaders/compiled/linux/spirv/vs_default.bin", "start": 98642841, "end": 98649552}, {"filename": "/GameData/shaders/compiled/linux/spirv/vs_fullscreen.bin", "start": 98649552, "end": 98650290}, {"filename": "/GameData/shaders/compiled/linux/spirv/vs_instanced_billboard.bin", "start": 98650290, "end": 98653599}, {"filename": "/GameData/shaders/compiled/linux/spirv/vs_ui.bin", "start": 98653599, "end": 98654925}, {"filename": "/GameData/shaders/compiled/web/gles/bsp/fs_bsp.bin", "start": 98654925, "end": 98660554}, {"filename": "/GameData/shaders/compiled/web/gles/bsp/fs_bsp_cube.bin", "start": 98660554, "end": 98661108}, {"filename": "/GameData/shaders/compiled/web/gles/bsp/fs_bsp_empty.bin", "start": 98661108, "end": 98661267}, {"filename": "/GameData/shaders/compiled/web/gles/bsp/vs_bsp.bin", "start": 98661267, "end": 98663762}, {"filename": "/GameData/shaders/compiled/web/gles/debugDraw/fs_debugdraw.bin", "start": 98663762, "end": 98663923}, {"filename": "/GameData/shaders/compiled/web/gles/debugDraw/vs_debugdraw.bin", "start": 98663923, "end": 98664282}, {"filename": "/GameData/shaders/compiled/web/gles/fs_bsp.bin", "start": 98664282, "end": 98669911}, {"filename": "/GameData/shaders/compiled/web/gles/fs_bsp_cube.bin", "start": 98669911, "end": 98670465}, {"filename": "/GameData/shaders/compiled/web/gles/fs_bsp_empty.bin", "start": 98670465, "end": 98670624}, {"filename": "/GameData/shaders/compiled/web/gles/fs_copy.bin", "start": 98670624, "end": 98670918}, {"filename": "/GameData/shaders/compiled/web/gles/fs_copy_depth.bin", "start": 98670918, "end": 98671176}, {"filename": "/GameData/shaders/compiled/web/gles/fs_customId.bin", "start": 98671176, "end": 98672051}, {"filename": "/GameData/shaders/compiled/web/gles/fs_default.bin", "start": 98672051, "end": 98679320}, {"filename": "/GameData/shaders/compiled/web/gles/fs_default_simple.bin", "start": 98679320, "end": 98683282}, {"filename": "/GameData/shaders/compiled/web/gles/fs_empty.bin", "start": 98683282, "end": 98683441}, {"filename": "/GameData/shaders/compiled/web/gles/fs_fxaa_simple.bin", "start": 98683441, "end": 98687388}, {"filename": "/GameData/shaders/compiled/web/gles/fs_mask.bin", "start": 98687388, "end": 98687698}, {"filename": "/GameData/shaders/compiled/web/gles/fs_motionBlur.bin", "start": 98687698, "end": 98689874}, {"filename": "/GameData/shaders/compiled/web/gles/fs_motionBlur_apply.bin", "start": 98689874, "end": 98690375}, {"filename": "/GameData/shaders/compiled/web/gles/fs_postprocessing.bin", "start": 98690375, "end": 98697710}, {"filename": "/GameData/shaders/compiled/web/gles/fs_resolve_depth_msaa.bin", "start": 98697710, "end": 98697813}, {"filename": "/GameData/shaders/compiled/web/gles/fs_solidRed.bin", "start": 98697813, "end": 98697972}, {"filename": "/GameData/shaders/compiled/web/gles/fs_ui_flatcolor.bin", "start": 98697972, "end": 98698154}, {"filename": "/GameData/shaders/compiled/web/gles/fs_ui_textured.bin", "start": 98698154, "end": 98698583}, {"filename": "/GameData/shaders/compiled/web/gles/fs_ui_textured_point.bin", "start": 98698583, "end": 98699137}, {"filename": "/GameData/shaders/compiled/web/gles/fs_unlit.bin", "start": 98699137, "end": 98699638}, {"filename": "/GameData/shaders/compiled/web/gles/fs_unlit_rect.bin", "start": 98699638, "end": 98700225}, {"filename": "/GameData/shaders/compiled/web/gles/game/fs_debuff.bin", "start": 98700225, "end": 98702806}, {"filename": "/GameData/shaders/compiled/web/gles/game/fs_inventory_menu_sphere.bin", "start": 98702806, "end": 98703036}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_blendmask.bin", "start": 98703036, "end": 98703457}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_blur.bin", "start": 98703457, "end": 98706666}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_color.bin", "start": 98706666, "end": 98706827}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_colormatrix.bin", "start": 98706827, "end": 98707555}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_creation.bin", "start": 98707555, "end": 98707714}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_dropshadow.bin", "start": 98707714, "end": 98708144}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_gradient.bin", "start": 98708144, "end": 98712285}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_passthrough.bin", "start": 98712285, "end": 98712570}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_texture.bin", "start": 98712570, "end": 98712886}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/vs_rmlui.bin", "start": 98712886, "end": 98713382}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/vs_rmlui_passthrough.bin", "start": 98713382, "end": 98713900}, {"filename": "/GameData/shaders/compiled/web/gles/ui/fs_progressBar.bin", "start": 98713900, "end": 98714590}, {"filename": "/GameData/shaders/compiled/web/gles/vs_bsp.bin", "start": 98714590, "end": 98717085}, {"filename": "/GameData/shaders/compiled/web/gles/vs_default.bin", "start": 98717085, "end": 98722065}, {"filename": "/GameData/shaders/compiled/web/gles/vs_fullscreen.bin", "start": 98722065, "end": 98722368}, {"filename": "/GameData/shaders/compiled/web/gles/vs_instanced_billboard.bin", "start": 98722368, "end": 98724068}, {"filename": "/GameData/shaders/compiled/web/gles/vs_ui.bin", "start": 98724068, "end": 98724502}, {"filename": "/GameData/shaders/compiled/web/spirv/bsp/fs_bsp.bin", "start": 98724502, "end": 98733381}, {"filename": "/GameData/shaders/compiled/web/spirv/bsp/fs_bsp_cube.bin", "start": 98733381, "end": 98734958}, {"filename": "/GameData/shaders/compiled/web/spirv/bsp/fs_bsp_empty.bin", "start": 98734958, "end": 98735320}, {"filename": "/GameData/shaders/compiled/web/spirv/bsp/vs_bsp.bin", "start": 98735320, "end": 98739034}, {"filename": "/GameData/shaders/compiled/web/spirv/debugDraw/fs_debugdraw.bin", "start": 98739034, "end": 98739440}, {"filename": "/GameData/shaders/compiled/web/spirv/debugDraw/vs_debugdraw.bin", "start": 98739440, "end": 98740500}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_bsp.bin", "start": 98740500, "end": 98749379}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_bsp_cube.bin", "start": 98749379, "end": 98750956}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_bsp_empty.bin", "start": 98750956, "end": 98751318}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_copy.bin", "start": 98751318, "end": 98752164}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_copy_depth.bin", "start": 98752164, "end": 98753033}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_customId.bin", "start": 98753033, "end": 98754898}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_default.bin", "start": 98754898, "end": 98763435}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_default_simple.bin", "start": 98763435, "end": 98769528}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_empty.bin", "start": 98769528, "end": 98769890}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_fxaa_simple.bin", "start": 98769890, "end": 98773557}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_mask.bin", "start": 98773557, "end": 98774443}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_motionBlur.bin", "start": 98774443, "end": 98778661}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_motionBlur_apply.bin", "start": 98778661, "end": 98780021}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_postprocessing.bin", "start": 98780021, "end": 98789073}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_resolve_depth_msaa.bin", "start": 98789073, "end": 98790600}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_solidRed.bin", "start": 98790600, "end": 98790978}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_ui_flatcolor.bin", "start": 98790978, "end": 98791562}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_ui_textured.bin", "start": 98791562, "end": 98792778}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_ui_textured_point.bin", "start": 98792778, "end": 98794322}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_unlit.bin", "start": 98794322, "end": 98795589}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_unlit_rect.bin", "start": 98795589, "end": 98797066}, {"filename": "/GameData/shaders/compiled/web/spirv/game/fs_debuff.bin", "start": 98797066, "end": 98800031}, {"filename": "/GameData/shaders/compiled/web/spirv/game/fs_inventory_menu_sphere.bin", "start": 98800031, "end": 98800425}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_blendmask.bin", "start": 98800425, "end": 98801465}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_blur.bin", "start": 98801465, "end": 98804093}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_color.bin", "start": 98804093, "end": 98804499}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_colormatrix.bin", "start": 98804499, "end": 98806469}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_creation.bin", "start": 98806469, "end": 98806831}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_dropshadow.bin", "start": 98806831, "end": 98808082}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_gradient.bin", "start": 98808082, "end": 98811310}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_passthrough.bin", "start": 98811310, "end": 98812065}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_texture.bin", "start": 98812065, "end": 98812928}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/vs_rmlui.bin", "start": 98812928, "end": 98814242}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/vs_rmlui_passthrough.bin", "start": 98814242, "end": 98815644}, {"filename": "/GameData/shaders/compiled/web/spirv/ui/fs_progressBar.bin", "start": 98815644, "end": 98817553}, {"filename": "/GameData/shaders/compiled/web/spirv/vs_bsp.bin", "start": 98817553, "end": 98821267}, {"filename": "/GameData/shaders/compiled/web/spirv/vs_default.bin", "start": 98821267, "end": 98827978}, {"filename": "/GameData/shaders/compiled/web/spirv/vs_fullscreen.bin", "start": 98827978, "end": 98828716}, {"filename": "/GameData/shaders/compiled/web/spirv/vs_instanced_billboard.bin", "start": 98828716, "end": 98832025}, {"filename": "/GameData/shaders/compiled/web/spirv/vs_ui.bin", "start": 98832025, "end": 98833351}, {"filename": "/GameData/shaders/compiled/windows/dx11/bsp/fs_bsp.bin", "start": 98833351, "end": 98839110}, {"filename": "/GameData/shaders/compiled/windows/dx11/bsp/fs_bsp_cube.bin", "start": 98839110, "end": 98840143}, {"filename": "/GameData/shaders/compiled/windows/dx11/bsp/fs_bsp_empty.bin", "start": 98840143, "end": 98840589}, {"filename": "/GameData/shaders/compiled/windows/dx11/bsp/vs_bsp.bin", "start": 98840589, "end": 98844551}, {"filename": "/GameData/shaders/compiled/windows/dx11/debugDraw/fs_debugdraw.bin", "start": 98844551, "end": 98844821}, {"filename": "/GameData/shaders/compiled/windows/dx11/debugDraw/vs_debugdraw.bin", "start": 98844821, "end": 98845465}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_bsp.bin", "start": 98845465, "end": 98851224}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_bsp_cube.bin", "start": 98851224, "end": 98852257}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_bsp_empty.bin", "start": 98852257, "end": 98852703}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_copy.bin", "start": 98852703, "end": 98853193}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_copy_depth.bin", "start": 98853193, "end": 98853694}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_customId.bin", "start": 98853694, "end": 98854931}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_default.bin", "start": 98854931, "end": 98866680}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_default_simple.bin", "start": 98866680, "end": 98871017}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_empty.bin", "start": 98871017, "end": 98871439}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_fxaa_simple.bin", "start": 98871439, "end": 98875166}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_mask.bin", "start": 98875166, "end": 98875740}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_motionBlur.bin", "start": 98875740, "end": 98879478}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_motionBlur_apply.bin", "start": 98879478, "end": 98880390}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_postprocessing.bin", "start": 98880390, "end": 98888986}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_resolve_depth_msaa.bin", "start": 98888986, "end": 98889961}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_solidRed.bin", "start": 98889961, "end": 98890411}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_ui_flatcolor.bin", "start": 98890411, "end": 98890711}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_ui_textured.bin", "start": 98890711, "end": 98891199}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_ui_textured_point.bin", "start": 98891199, "end": 98891883}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_unlit.bin", "start": 98891883, "end": 98892630}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_unlit_rect.bin", "start": 98892630, "end": 98893615}, {"filename": "/GameData/shaders/compiled/windows/dx11/game/fs_debuff.bin", "start": 98893615, "end": 98897456}, {"filename": "/GameData/shaders/compiled/windows/dx11/game/fs_inventory_menu_sphere.bin", "start": 98897456, "end": 98897946}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_blendmask.bin", "start": 98897946, "end": 98898534}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_blur.bin", "start": 98898534, "end": 98900450}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_color.bin", "start": 98900450, "end": 98900776}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_colormatrix.bin", "start": 98900776, "end": 98901734}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_creation.bin", "start": 98901734, "end": 98902060}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_dropshadow.bin", "start": 98902060, "end": 98902699}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_gradient.bin", "start": 98902699, "end": 98906707}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_passthrough.bin", "start": 98906707, "end": 98907134}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_texture.bin", "start": 98907134, "end": 98907601}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/vs_rmlui.bin", "start": 98907601, "end": 98908479}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/vs_rmlui_passthrough.bin", "start": 98908479, "end": 98909269}, {"filename": "/GameData/shaders/compiled/windows/dx11/ui/fs_progressBar.bin", "start": 98909269, "end": 98909990}, {"filename": "/GameData/shaders/compiled/windows/dx11/vs_bsp.bin", "start": 98909990, "end": 98913952}, {"filename": "/GameData/shaders/compiled/windows/dx11/vs_default.bin", "start": 98913952, "end": 98924263}, {"filename": "/GameData/shaders/compiled/windows/dx11/vs_fullscreen.bin", "start": 98924263, "end": 98924701}, {"filename": "/GameData/shaders/compiled/windows/dx11/vs_instanced_billboard.bin", "start": 98924701, "end": 98927362}, {"filename": "/GameData/shaders/compiled/windows/dx11/vs_ui.bin", "start": 98927362, "end": 98928253}, {"filename": "/GameData/shaders/compiled/windows/dx12/bsp/fs_bsp.bin", "start": 98928253, "end": 98934012}, {"filename": "/GameData/shaders/compiled/windows/dx12/bsp/fs_bsp_cube.bin", "start": 98934012, "end": 98935045}, {"filename": "/GameData/shaders/compiled/windows/dx12/bsp/fs_bsp_empty.bin", "start": 98935045, "end": 98935491}, {"filename": "/GameData/shaders/compiled/windows/dx12/bsp/vs_bsp.bin", "start": 98935491, "end": 98939453}, {"filename": "/GameData/shaders/compiled/windows/dx12/debugDraw/fs_debugdraw.bin", "start": 98939453, "end": 98939723}, {"filename": "/GameData/shaders/compiled/windows/dx12/debugDraw/vs_debugdraw.bin", "start": 98939723, "end": 98940367}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_bsp.bin", "start": 98940367, "end": 98946126}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_bsp_cube.bin", "start": 98946126, "end": 98947159}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_bsp_empty.bin", "start": 98947159, "end": 98947605}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_copy.bin", "start": 98947605, "end": 98948095}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_copy_depth.bin", "start": 98948095, "end": 98948596}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_customId.bin", "start": 98948596, "end": 98949833}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_default.bin", "start": 98949833, "end": 98961582}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_default_simple.bin", "start": 98961582, "end": 98965919}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_empty.bin", "start": 98965919, "end": 98966341}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_fxaa_simple.bin", "start": 98966341, "end": 98970068}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_mask.bin", "start": 98970068, "end": 98970642}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_motionBlur.bin", "start": 98970642, "end": 98974380}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_motionBlur_apply.bin", "start": 98974380, "end": 98975292}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_postprocessing.bin", "start": 98975292, "end": 98983888}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_resolve_depth_msaa.bin", "start": 98983888, "end": 98984863}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_solidRed.bin", "start": 98984863, "end": 98985313}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_ui_flatcolor.bin", "start": 98985313, "end": 98985613}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_ui_textured.bin", "start": 98985613, "end": 98986101}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_ui_textured_point.bin", "start": 98986101, "end": 98986785}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_unlit.bin", "start": 98986785, "end": 98987532}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_unlit_rect.bin", "start": 98987532, "end": 98988517}, {"filename": "/GameData/shaders/compiled/windows/dx12/game/fs_debuff.bin", "start": 98988517, "end": 98992358}, {"filename": "/GameData/shaders/compiled/windows/dx12/game/fs_inventory_menu_sphere.bin", "start": 98992358, "end": 98992848}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_blendmask.bin", "start": 98992848, "end": 98993436}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_blur.bin", "start": 98993436, "end": 98995352}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_color.bin", "start": 98995352, "end": 98995678}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_colormatrix.bin", "start": 98995678, "end": 98996636}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_creation.bin", "start": 98996636, "end": 98996962}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_dropshadow.bin", "start": 98996962, "end": 98997601}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_gradient.bin", "start": 98997601, "end": 99001609}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_passthrough.bin", "start": 99001609, "end": 99002036}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_texture.bin", "start": 99002036, "end": 99002503}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/vs_rmlui.bin", "start": 99002503, "end": 99003381}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/vs_rmlui_passthrough.bin", "start": 99003381, "end": 99004171}, {"filename": "/GameData/shaders/compiled/windows/dx12/ui/fs_progressBar.bin", "start": 99004171, "end": 99004892}, {"filename": "/GameData/shaders/compiled/windows/dx12/vs_bsp.bin", "start": 99004892, "end": 99008854}, {"filename": "/GameData/shaders/compiled/windows/dx12/vs_default.bin", "start": 99008854, "end": 99019165}, {"filename": "/GameData/shaders/compiled/windows/dx12/vs_fullscreen.bin", "start": 99019165, "end": 99019603}, {"filename": "/GameData/shaders/compiled/windows/dx12/vs_instanced_billboard.bin", "start": 99019603, "end": 99022264}, {"filename": "/GameData/shaders/compiled/windows/dx12/vs_ui.bin", "start": 99022264, "end": 99023155}, {"filename": "/GameData/shaders/compiled/windows/gl/bsp/fs_bsp.bin", "start": 99023155, "end": 99028606}, {"filename": "/GameData/shaders/compiled/windows/gl/bsp/fs_bsp_cube.bin", "start": 99028606, "end": 99029031}, {"filename": "/GameData/shaders/compiled/windows/gl/bsp/fs_bsp_empty.bin", "start": 99029031, "end": 99029111}, {"filename": "/GameData/shaders/compiled/windows/gl/bsp/vs_bsp.bin", "start": 99029111, "end": 99031406}, {"filename": "/GameData/shaders/compiled/windows/gl/debugDraw/fs_debugdraw.bin", "start": 99031406, "end": 99031488}, {"filename": "/GameData/shaders/compiled/windows/gl/debugDraw/vs_debugdraw.bin", "start": 99031488, "end": 99031773}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_bsp.bin", "start": 99031773, "end": 99037224}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_bsp_cube.bin", "start": 99037224, "end": 99037649}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_bsp_empty.bin", "start": 99037649, "end": 99037729}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_copy.bin", "start": 99037729, "end": 99037899}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_copy_depth.bin", "start": 99037899, "end": 99038068}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_customId.bin", "start": 99038068, "end": 99038851}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_default.bin", "start": 99038851, "end": 99045958}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_default_simple.bin", "start": 99045958, "end": 99049753}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_empty.bin", "start": 99049753, "end": 99049833}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_fxaa_simple.bin", "start": 99049833, "end": 99053291}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_mask.bin", "start": 99053291, "end": 99053552}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_motionBlur.bin", "start": 99053552, "end": 99055576}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_motionBlur_apply.bin", "start": 99055576, "end": 99055988}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_postprocessing.bin", "start": 99055988, "end": 99062805}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_resolve_depth_msaa.bin", "start": 99062805, "end": 99063381}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_solidRed.bin", "start": 99063381, "end": 99063461}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_ui_flatcolor.bin", "start": 99063461, "end": 99063564}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_ui_textured.bin", "start": 99063564, "end": 99063904}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_ui_textured_point.bin", "start": 99063904, "end": 99064369}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_unlit.bin", "start": 99064369, "end": 99064781}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_unlit_rect.bin", "start": 99064781, "end": 99065279}, {"filename": "/GameData/shaders/compiled/windows/gl/game/fs_debuff.bin", "start": 99065279, "end": 99067756}, {"filename": "/GameData/shaders/compiled/windows/gl/game/fs_inventory_menu_sphere.bin", "start": 99067756, "end": 99067899}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_blendmask.bin", "start": 99067899, "end": 99068151}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_blur.bin", "start": 99068151, "end": 99071276}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_color.bin", "start": 99071276, "end": 99071358}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_colormatrix.bin", "start": 99071358, "end": 99071987}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_creation.bin", "start": 99071987, "end": 99072067}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_dropshadow.bin", "start": 99072067, "end": 99072373}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_gradient.bin", "start": 99072373, "end": 99076419}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_passthrough.bin", "start": 99076419, "end": 99076580}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_texture.bin", "start": 99076580, "end": 99076772}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/vs_rmlui.bin", "start": 99076772, "end": 99077176}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/vs_rmlui_passthrough.bin", "start": 99077176, "end": 99077602}, {"filename": "/GameData/shaders/compiled/windows/gl/ui/fs_progressBar.bin", "start": 99077602, "end": 99078203}, {"filename": "/GameData/shaders/compiled/windows/gl/vs_bsp.bin", "start": 99078203, "end": 99080498}, {"filename": "/GameData/shaders/compiled/windows/gl/vs_default.bin", "start": 99080498, "end": 99085164}, {"filename": "/GameData/shaders/compiled/windows/gl/vs_fullscreen.bin", "start": 99085164, "end": 99085399}, {"filename": "/GameData/shaders/compiled/windows/gl/vs_instanced_billboard.bin", "start": 99085399, "end": 99086899}, {"filename": "/GameData/shaders/compiled/windows/gl/vs_ui.bin", "start": 99086899, "end": 99087253}, {"filename": "/GameData/shaders/compiled/windows/spirv/bsp/fs_bsp.bin", "start": 99087253, "end": 99096132}, {"filename": "/GameData/shaders/compiled/windows/spirv/bsp/fs_bsp_cube.bin", "start": 99096132, "end": 99097709}, {"filename": "/GameData/shaders/compiled/windows/spirv/bsp/fs_bsp_empty.bin", "start": 99097709, "end": 99098071}, {"filename": "/GameData/shaders/compiled/windows/spirv/bsp/vs_bsp.bin", "start": 99098071, "end": 99101785}, {"filename": "/GameData/shaders/compiled/windows/spirv/debugDraw/fs_debugdraw.bin", "start": 99101785, "end": 99102191}, {"filename": "/GameData/shaders/compiled/windows/spirv/debugDraw/vs_debugdraw.bin", "start": 99102191, "end": 99103251}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_bsp.bin", "start": 99103251, "end": 99112130}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_bsp_cube.bin", "start": 99112130, "end": 99113707}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_bsp_empty.bin", "start": 99113707, "end": 99114069}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_copy.bin", "start": 99114069, "end": 99114915}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_copy_depth.bin", "start": 99114915, "end": 99115784}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_customId.bin", "start": 99115784, "end": 99117649}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_default.bin", "start": 99117649, "end": 99126186}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_default_simple.bin", "start": 99126186, "end": 99132279}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_empty.bin", "start": 99132279, "end": 99132641}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_fxaa_simple.bin", "start": 99132641, "end": 99136308}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_mask.bin", "start": 99136308, "end": 99137194}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_motionBlur.bin", "start": 99137194, "end": 99141412}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_motionBlur_apply.bin", "start": 99141412, "end": 99142772}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_postprocessing.bin", "start": 99142772, "end": 99151824}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_resolve_depth_msaa.bin", "start": 99151824, "end": 99153351}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_solidRed.bin", "start": 99153351, "end": 99153729}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_ui_flatcolor.bin", "start": 99153729, "end": 99154313}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_ui_textured.bin", "start": 99154313, "end": 99155529}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_ui_textured_point.bin", "start": 99155529, "end": 99157073}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_unlit.bin", "start": 99157073, "end": 99158340}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_unlit_rect.bin", "start": 99158340, "end": 99159817}, {"filename": "/GameData/shaders/compiled/windows/spirv/game/fs_debuff.bin", "start": 99159817, "end": 99162782}, {"filename": "/GameData/shaders/compiled/windows/spirv/game/fs_inventory_menu_sphere.bin", "start": 99162782, "end": 99163176}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_blendmask.bin", "start": 99163176, "end": 99164216}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_blur.bin", "start": 99164216, "end": 99166844}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_color.bin", "start": 99166844, "end": 99167250}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_colormatrix.bin", "start": 99167250, "end": 99169220}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_creation.bin", "start": 99169220, "end": 99169582}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_dropshadow.bin", "start": 99169582, "end": 99170833}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_gradient.bin", "start": 99170833, "end": 99174061}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_passthrough.bin", "start": 99174061, "end": 99174816}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_texture.bin", "start": 99174816, "end": 99175679}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/vs_rmlui.bin", "start": 99175679, "end": 99176993}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/vs_rmlui_passthrough.bin", "start": 99176993, "end": 99178395}, {"filename": "/GameData/shaders/compiled/windows/spirv/ui/fs_progressBar.bin", "start": 99178395, "end": 99180304}, {"filename": "/GameData/shaders/compiled/windows/spirv/vs_bsp.bin", "start": 99180304, "end": 99184018}, {"filename": "/GameData/shaders/compiled/windows/spirv/vs_default.bin", "start": 99184018, "end": 99190729}, {"filename": "/GameData/shaders/compiled/windows/spirv/vs_fullscreen.bin", "start": 99190729, "end": 99191467}, {"filename": "/GameData/shaders/compiled/windows/spirv/vs_instanced_billboard.bin", "start": 99191467, "end": 99194776}, {"filename": "/GameData/shaders/compiled/windows/spirv/vs_ui.bin", "start": 99194776, "end": 99196102}, {"filename": "/GameData/shaders/source/bgfx_compute.sh", "start": 99196102, "end": 99214523}, {"filename": "/GameData/shaders/source/bgfx_shader.sh", "start": 99214523, "end": 99242482}, {"filename": "/GameData/shaders/source/bsp/bgfx_shader.sh", "start": 99242482, "end": 99270441}, {"filename": "/GameData/shaders/source/bsp/fs_bsp.sc", "start": 99270441, "end": 99274589}, {"filename": "/GameData/shaders/source/bsp/fs_bsp_cube.sc", "start": 99274589, "end": 99275031}, {"filename": "/GameData/shaders/source/bsp/fs_bsp_empty.sc", "start": 99275031, "end": 99275248}, {"filename": "/GameData/shaders/source/bsp/varying.def.sc", "start": 99275248, "end": 99276216}, {"filename": "/GameData/shaders/source/bsp/vs_bsp.sc", "start": 99276216, "end": 99277490}, {"filename": "/GameData/shaders/source/debugDraw/bgfx_shader.sh", "start": 99277490, "end": 99305449}, {"filename": "/GameData/shaders/source/debugDraw/fs_debugdraw.sc", "start": 99305449, "end": 99305538}, {"filename": "/GameData/shaders/source/debugDraw/varying.def.sc", "start": 99305538, "end": 99305651}, {"filename": "/GameData/shaders/source/debugDraw/vs_debugdraw.sc", "start": 99305651, "end": 99306052}, {"filename": "/GameData/shaders/source/fs_copy.sc", "start": 99306052, "end": 99306306}, {"filename": "/GameData/shaders/source/fs_copy_depth.sc", "start": 99306306, "end": 99306556}, {"filename": "/GameData/shaders/source/fs_customId.sc", "start": 99306556, "end": 99307263}, {"filename": "/GameData/shaders/source/fs_default.sc", "start": 99307263, "end": 99316058}, {"filename": "/GameData/shaders/source/fs_default_simple.sc", "start": 99316058, "end": 99319629}, {"filename": "/GameData/shaders/source/fs_empty.sc", "start": 99319629, "end": 99319831}, {"filename": "/GameData/shaders/source/fs_fxaa_simple.sc", "start": 99319831, "end": 99324021}, {"filename": "/GameData/shaders/source/fs_mask.sc", "start": 99324021, "end": 99324345}, {"filename": "/GameData/shaders/source/fs_motionBlur.sc", "start": 99324345, "end": 99326890}, {"filename": "/GameData/shaders/source/fs_motionBlur_apply.sc", "start": 99326890, "end": 99327483}, {"filename": "/GameData/shaders/source/fs_postprocessing.sc", "start": 99327483, "end": 99334752}, {"filename": "/GameData/shaders/source/fs_resolve_depth_msaa.sc", "start": 99334752, "end": 99335649}, {"filename": "/GameData/shaders/source/fs_solidRed.sc", "start": 99335649, "end": 99335869}, {"filename": "/GameData/shaders/source/fs_ui_flatcolor.sc", "start": 99335869, "end": 99335993}, {"filename": "/GameData/shaders/source/fs_ui_textured.sc", "start": 99335993, "end": 99336279}, {"filename": "/GameData/shaders/source/fs_ui_textured_point.sc", "start": 99336279, "end": 99336719}, {"filename": "/GameData/shaders/source/fs_unlit.sc", "start": 99336719, "end": 99337116}, {"filename": "/GameData/shaders/source/fs_unlit_rect.sc", "start": 99337116, "end": 99337725}, {"filename": "/GameData/shaders/source/game/fs_debuff.sc", "start": 99337725, "end": 99340554}, {"filename": "/GameData/shaders/source/game/fs_inventory_menu_sphere.sc", "start": 99340554, "end": 99341165}, {"filename": "/GameData/shaders/source/rmlui/bgfx_shader.sh", "start": 99341165, "end": 99369124}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_blendmask.sc", "start": 99369124, "end": 99369754}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_blur.sc", "start": 99369754, "end": 99371392}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_color.sc", "start": 99371392, "end": 99371683}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_colormatrix.sc", "start": 99371683, "end": 99372755}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_creation.sc", "start": 99372755, "end": 99373384}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_dropshadow.sc", "start": 99373384, "end": 99374275}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_gradient.sc", "start": 99374275, "end": 99377253}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_passthrough.sc", "start": 99377253, "end": 99377622}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_texture.sc", "start": 99377622, "end": 99378050}, {"filename": "/GameData/shaders/source/rmlui/varying.def.sc", "start": 99378050, "end": 99378294}, {"filename": "/GameData/shaders/source/rmlui/vs_rmlui.sc", "start": 99378294, "end": 99379105}, {"filename": "/GameData/shaders/source/rmlui/vs_rmlui_passthrough.sc", "start": 99379105, "end": 99379967}, {"filename": "/GameData/shaders/source/ui/fs_progressBar.sc", "start": 99379967, "end": 99380701}, {"filename": "/GameData/shaders/source/varying.def.sc", "start": 99380701, "end": 99381677}, {"filename": "/GameData/shaders/source/vs_default.sc", "start": 99381677, "end": 99385175}, {"filename": "/GameData/shaders/source/vs_fullscreen.sc", "start": 99385175, "end": 99385360}, {"filename": "/GameData/shaders/source/vs_instanced_billboard.sc", "start": 99385360, "end": 99386537}, {"filename": "/GameData/shaders/source/vs_ui.sc", "start": 99386537, "end": 99386986}, {"filename": "/GameData/sounds/banks/Desktop/Dialogue_CN.bank", "start": 99386986, "end": 99439402}, {"filename": "/GameData/sounds/banks/Desktop/Dialogue_EN.bank", "start": 99439402, "end": 99500522}, {"filename": "/GameData/sounds/banks/Desktop/Dialogue_JP.bank", "start": 99500522, "end": 99567082}, {"filename": "/GameData/sounds/banks/Desktop/Master.bank", "start": 99567082, "end": 99767274}, {"filename": "/GameData/sounds/banks/Desktop/Master.strings.bank", "start": 99767274, "end": 99771954}, {"filename": "/GameData/sounds/banks/Desktop/Music.bank", "start": 99771954, "end": 102309778}, {"filename": "/GameData/sounds/banks/Desktop/SFX.bank", "start": 102309778, "end": 124885810}, {"filename": "/GameData/sounds/banks/Desktop/VO.bank", "start": 124885810, "end": 125150066}, {"filename": "/GameData/sounds/banks/Desktop/Weapons.bank", "start": 125150066, "end": 125265554}, {"filename": "/GameData/sounds/dog/death.wav", "start": 125265554, "end": 125338940, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_attack.wav", "start": 125338940, "end": 125451464, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_attack_start.wav", "start": 125451464, "end": 125648148, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_death.wav", "start": 125648148, "end": 125802348, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_hit.wav", "start": 125802348, "end": 125876040, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_stun.wav", "start": 125876040, "end": 125969490, "audio": 1}, {"filename": "/GameData/sounds/mew.wav", "start": 125969490, "end": 126042876, "audio": 1}, {"filename": "/GameData/sounds/weapons/shotgun/shotgun_fire.wav", "start": 126042876, "end": 126503760, "audio": 1}, {"filename": "/GameData/sounds/weapons/shotgun/shotgun_fire2.wav", "start": 126503760, "end": 126964644, "audio": 1}, {"filename": "/GameData/tables/items/items.csv", "start": 126964644, "end": 126966005}, {"filename": "/GameData/tables/npc/npc_example.csv", "start": 126966005, "end": 126966693}, {"filename": "/GameData/testViewmodel.glb", "start": 126966693, "end": 127838209}, {"filename": "/GameData/textures/FPSTextures/BigWall_01.png", "start": 127838209, "end": 127849636}, {"filename": "/GameData/textures/FPSTextures/BigWall_02.png", "start": 127849636, "end": 127856906}, {"filename": "/GameData/textures/FPSTextures/BigWall_03.png", "start": 127856906, "end": 127866574}, {"filename": "/GameData/textures/FPSTextures/BigWall_04.png", "start": 127866574, "end": 127881321}, {"filename": "/GameData/textures/FPSTextures/BigWall_05.png", "start": 127881321, "end": 127886704}, {"filename": "/GameData/textures/FPSTextures/Column01.png", "start": 127886704, "end": 127890374}, {"filename": "/GameData/textures/FPSTextures/Column02.png", "start": 127890374, "end": 127892515}, {"filename": "/GameData/textures/FPSTextures/Column03.png", "start": 127892515, "end": 127895917}, {"filename": "/GameData/textures/FPSTextures/Column04.png", "start": 127895917, "end": 127902115}, {"filename": "/GameData/textures/FPSTextures/Column05.png", "start": 127902115, "end": 127904599}, {"filename": "/GameData/textures/FPSTextures/Column06.png", "start": 127904599, "end": 127910383}, {"filename": "/GameData/textures/FPSTextures/Column07.png", "start": 127910383, "end": 127916359}, {"filename": "/GameData/textures/FPSTextures/Column08.png", "start": 127916359, "end": 127922530}, {"filename": "/GameData/textures/FPSTextures/Column09.png", "start": 127922530, "end": 127929058}, {"filename": "/GameData/textures/FPSTextures/Column10.png", "start": 127929058, "end": 127930830}, {"filename": "/GameData/textures/FPSTextures/Column11.png", "start": 127930830, "end": 127935502}, {"filename": "/GameData/textures/FPSTextures/Column12.png", "start": 127935502, "end": 127941318}, {"filename": "/GameData/textures/FPSTextures/Column_13.png", "start": 127941318, "end": 127943469}, {"filename": "/GameData/textures/FPSTextures/Column_14.png", "start": 127943469, "end": 127945236}, {"filename": "/GameData/textures/FPSTextures/Column_15.png", "start": 127945236, "end": 127947203}, {"filename": "/GameData/textures/FPSTextures/Column_16.png", "start": 127947203, "end": 127949884}, {"filename": "/GameData/textures/FPSTextures/Column_17.png", "start": 127949884, "end": 127952512}, {"filename": "/GameData/textures/FPSTextures/Column_18.png", "start": 127952512, "end": 127955326}, {"filename": "/GameData/textures/FPSTextures/Crate01.png", "start": 127955326, "end": 127961532}, {"filename": "/GameData/textures/FPSTextures/Crate02.png", "start": 127961532, "end": 127967600}, {"filename": "/GameData/textures/FPSTextures/Crate03.png", "start": 127967600, "end": 127970110}, {"filename": "/GameData/textures/FPSTextures/Door_01.png", "start": 127970110, "end": 127977185}, {"filename": "/GameData/textures/FPSTextures/Door_02.png", "start": 127977185, "end": 127986976}, {"filename": "/GameData/textures/FPSTextures/Door_03.png", "start": 127986976, "end": 127999152}, {"filename": "/GameData/textures/FPSTextures/Door_04.png", "start": 127999152, "end": 128007370}, {"filename": "/GameData/textures/FPSTextures/Flat01.png", "start": 128007370, "end": 128010033}, {"filename": "/GameData/textures/FPSTextures/Flat02.png", "start": 128010033, "end": 128012787}, {"filename": "/GameData/textures/FPSTextures/Flat03.png", "start": 128012787, "end": 128017362}, {"filename": "/GameData/textures/FPSTextures/Flat04.png", "start": 128017362, "end": 128021618}, {"filename": "/GameData/textures/FPSTextures/Flat05.png", "start": 128021618, "end": 128024060}, {"filename": "/GameData/textures/FPSTextures/Flat06.png", "start": 128024060, "end": 128026145}, {"filename": "/GameData/textures/FPSTextures/Flat07.png", "start": 128026145, "end": 128031567}, {"filename": "/GameData/textures/FPSTextures/Flat08.png", "start": 128031567, "end": 128037416}, {"filename": "/GameData/textures/FPSTextures/Flat09.png", "start": 128037416, "end": 128042035}, {"filename": "/GameData/textures/FPSTextures/Flat10.png", "start": 128042035, "end": 128046981}, {"filename": "/GameData/textures/FPSTextures/Flat100.png", "start": 128046981, "end": 128052893}, {"filename": "/GameData/textures/FPSTextures/Flat101.png", "start": 128052893, "end": 128055680}, {"filename": "/GameData/textures/FPSTextures/Flat102.png", "start": 128055680, "end": 128057846}, {"filename": "/GameData/textures/FPSTextures/Flat103.png", "start": 128057846, "end": 128059393}, {"filename": "/GameData/textures/FPSTextures/Flat104.png", "start": 128059393, "end": 128061395}, {"filename": "/GameData/textures/FPSTextures/Flat105.png", "start": 128061395, "end": 128063066}, {"filename": "/GameData/textures/FPSTextures/Flat106.png", "start": 128063066, "end": 128065189}, {"filename": "/GameData/textures/FPSTextures/Flat107.png", "start": 128065189, "end": 128067567}, {"filename": "/GameData/textures/FPSTextures/Flat108.png", "start": 128067567, "end": 128069404}, {"filename": "/GameData/textures/FPSTextures/Flat109.png", "start": 128069404, "end": 128074096}, {"filename": "/GameData/textures/FPSTextures/Flat11.png", "start": 128074096, "end": 128076721}, {"filename": "/GameData/textures/FPSTextures/Flat110.png", "start": 128076721, "end": 128078923}, {"filename": "/GameData/textures/FPSTextures/Flat111.png", "start": 128078923, "end": 128084256}, {"filename": "/GameData/textures/FPSTextures/Flat112.png", "start": 128084256, "end": 128086147}, {"filename": "/GameData/textures/FPSTextures/Flat113.png", "start": 128086147, "end": 128088301}, {"filename": "/GameData/textures/FPSTextures/Flat114.png", "start": 128088301, "end": 128090184}, {"filename": "/GameData/textures/FPSTextures/Flat115.png", "start": 128090184, "end": 128092950}, {"filename": "/GameData/textures/FPSTextures/Flat116.png", "start": 128092950, "end": 128098525}, {"filename": "/GameData/textures/FPSTextures/Flat117.png", "start": 128098525, "end": 128104655}, {"filename": "/GameData/textures/FPSTextures/Flat118.png", "start": 128104655, "end": 128109747}, {"filename": "/GameData/textures/FPSTextures/Flat119.png", "start": 128109747, "end": 128112374}, {"filename": "/GameData/textures/FPSTextures/Flat12.png", "start": 128112374, "end": 128114956}, {"filename": "/GameData/textures/FPSTextures/Flat120.png", "start": 128114956, "end": 128119342}, {"filename": "/GameData/textures/FPSTextures/Flat121.png", "start": 128119342, "end": 128124870}, {"filename": "/GameData/textures/FPSTextures/Flat122.png", "start": 128124870, "end": 128130280}, {"filename": "/GameData/textures/FPSTextures/Flat123.png", "start": 128130280, "end": 128136076}, {"filename": "/GameData/textures/FPSTextures/Flat124.png", "start": 128136076, "end": 128137536}, {"filename": "/GameData/textures/FPSTextures/Flat125.png", "start": 128137536, "end": 128142093}, {"filename": "/GameData/textures/FPSTextures/Flat126.png", "start": 128142093, "end": 128148295}, {"filename": "/GameData/textures/FPSTextures/Flat127.png", "start": 128148295, "end": 128154345}, {"filename": "/GameData/textures/FPSTextures/Flat128.png", "start": 128154345, "end": 128156552}, {"filename": "/GameData/textures/FPSTextures/Flat129.png", "start": 128156552, "end": 128159399}, {"filename": "/GameData/textures/FPSTextures/Flat13.png", "start": 128159399, "end": 128165045}, {"filename": "/GameData/textures/FPSTextures/Flat130.png", "start": 128165045, "end": 128171135}, {"filename": "/GameData/textures/FPSTextures/Flat131.png", "start": 128171135, "end": 128173405}, {"filename": "/GameData/textures/FPSTextures/Flat132.png", "start": 128173405, "end": 128174973}, {"filename": "/GameData/textures/FPSTextures/Flat133.png", "start": 128174973, "end": 128178618}, {"filename": "/GameData/textures/FPSTextures/Flat134.png", "start": 128178618, "end": 128188179}, {"filename": "/GameData/textures/FPSTextures/Flat135.png", "start": 128188179, "end": 128194544}, {"filename": "/GameData/textures/FPSTextures/Flat136.png", "start": 128194544, "end": 128197150}, {"filename": "/GameData/textures/FPSTextures/Flat137.png", "start": 128197150, "end": 128199654}, {"filename": "/GameData/textures/FPSTextures/Flat138.png", "start": 128199654, "end": 128204531}, {"filename": "/GameData/textures/FPSTextures/Flat139.png", "start": 128204531, "end": 128206147}, {"filename": "/GameData/textures/FPSTextures/Flat14.png", "start": 128206147, "end": 128207919}, {"filename": "/GameData/textures/FPSTextures/Flat140.png", "start": 128207919, "end": 128215425}, {"filename": "/GameData/textures/FPSTextures/Flat141.png", "start": 128215425, "end": 128218618}, {"filename": "/GameData/textures/FPSTextures/Flat142.png", "start": 128218618, "end": 128224431}, {"filename": "/GameData/textures/FPSTextures/Flat15.png", "start": 128224431, "end": 128228180}, {"filename": "/GameData/textures/FPSTextures/Flat16.png", "start": 128228180, "end": 128232125}, {"filename": "/GameData/textures/FPSTextures/Flat17.png", "start": 128232125, "end": 128234176}, {"filename": "/GameData/textures/FPSTextures/Flat18.png", "start": 128234176, "end": 128239566}, {"filename": "/GameData/textures/FPSTextures/Flat19.png", "start": 128239566, "end": 128244426}, {"filename": "/GameData/textures/FPSTextures/Flat20.png", "start": 128244426, "end": 128250798}, {"filename": "/GameData/textures/FPSTextures/Flat21.png", "start": 128250798, "end": 128255822}, {"filename": "/GameData/textures/FPSTextures/Flat22.png", "start": 128255822, "end": 128259099}, {"filename": "/GameData/textures/FPSTextures/Flat23.png", "start": 128259099, "end": 128261970}, {"filename": "/GameData/textures/FPSTextures/Flat24.png", "start": 128261970, "end": 128264468}, {"filename": "/GameData/textures/FPSTextures/Flat25.png", "start": 128264468, "end": 128267147}, {"filename": "/GameData/textures/FPSTextures/Flat26.png", "start": 128267147, "end": 128271787}, {"filename": "/GameData/textures/FPSTextures/Flat27.png", "start": 128271787, "end": 128278334}, {"filename": "/GameData/textures/FPSTextures/Flat28.png", "start": 128278334, "end": 128284113}, {"filename": "/GameData/textures/FPSTextures/Flat29.png", "start": 128284113, "end": 128289851}, {"filename": "/GameData/textures/FPSTextures/Flat30.png", "start": 128289851, "end": 128291819}, {"filename": "/GameData/textures/FPSTextures/Flat31.png", "start": 128291819, "end": 128294320}, {"filename": "/GameData/textures/FPSTextures/Flat32.png", "start": 128294320, "end": 128299805}, {"filename": "/GameData/textures/FPSTextures/Flat33.png", "start": 128299805, "end": 128302180}, {"filename": "/GameData/textures/FPSTextures/Flat34.png", "start": 128302180, "end": 128304549}, {"filename": "/GameData/textures/FPSTextures/Flat35.png", "start": 128304549, "end": 128306642}, {"filename": "/GameData/textures/FPSTextures/Flat36.png", "start": 128306642, "end": 128308532}, {"filename": "/GameData/textures/FPSTextures/Flat37.png", "start": 128308532, "end": 128314399}, {"filename": "/GameData/textures/FPSTextures/Flat38.png", "start": 128314399, "end": 128320247}, {"filename": "/GameData/textures/FPSTextures/Flat39.png", "start": 128320247, "end": 128325668}, {"filename": "/GameData/textures/FPSTextures/Flat40.png", "start": 128325668, "end": 128331394}, {"filename": "/GameData/textures/FPSTextures/Flat41.png", "start": 128331394, "end": 128333160}, {"filename": "/GameData/textures/FPSTextures/Flat42.png", "start": 128333160, "end": 128339428}, {"filename": "/GameData/textures/FPSTextures/Flat43.png", "start": 128339428, "end": 128342082}, {"filename": "/GameData/textures/FPSTextures/Flat44.png", "start": 128342082, "end": 128348033}, {"filename": "/GameData/textures/FPSTextures/Flat45.png", "start": 128348033, "end": 128350678}, {"filename": "/GameData/textures/FPSTextures/Flat46.png", "start": 128350678, "end": 128352465}, {"filename": "/GameData/textures/FPSTextures/Flat47.png", "start": 128352465, "end": 128354374}, {"filename": "/GameData/textures/FPSTextures/Flat48.png", "start": 128354374, "end": 128356791}, {"filename": "/GameData/textures/FPSTextures/Flat49.png", "start": 128356791, "end": 128359693}, {"filename": "/GameData/textures/FPSTextures/Flat50.png", "start": 128359693, "end": 128365654}, {"filename": "/GameData/textures/FPSTextures/Flat51.png", "start": 128365654, "end": 128371181}, {"filename": "/GameData/textures/FPSTextures/Flat52.png", "start": 128371181, "end": 128373117}, {"filename": "/GameData/textures/FPSTextures/Flat53.png", "start": 128373117, "end": 128378644}, {"filename": "/GameData/textures/FPSTextures/Flat54.png", "start": 128378644, "end": 128385486}, {"filename": "/GameData/textures/FPSTextures/Flat55.png", "start": 128385486, "end": 128387647}, {"filename": "/GameData/textures/FPSTextures/Flat56.png", "start": 128387647, "end": 128394314}, {"filename": "/GameData/textures/FPSTextures/Flat57.png", "start": 128394314, "end": 128399962}, {"filename": "/GameData/textures/FPSTextures/Flat58.png", "start": 128399962, "end": 128401952}, {"filename": "/GameData/textures/FPSTextures/Flat59.png", "start": 128401952, "end": 128403881}, {"filename": "/GameData/textures/FPSTextures/Flat60.png", "start": 128403881, "end": 128405690}, {"filename": "/GameData/textures/FPSTextures/Flat61.png", "start": 128405690, "end": 128408260}, {"filename": "/GameData/textures/FPSTextures/Flat62.png", "start": 128408260, "end": 128414427}, {"filename": "/GameData/textures/FPSTextures/Flat63.png", "start": 128414427, "end": 128420448}, {"filename": "/GameData/textures/FPSTextures/Flat64.png", "start": 128420448, "end": 128426183}, {"filename": "/GameData/textures/FPSTextures/Flat65.png", "start": 128426183, "end": 128429071}, {"filename": "/GameData/textures/FPSTextures/Flat66.png", "start": 128429071, "end": 128430998}, {"filename": "/GameData/textures/FPSTextures/Flat67.png", "start": 128430998, "end": 128433031}, {"filename": "/GameData/textures/FPSTextures/Flat68.png", "start": 128433031, "end": 128435193}, {"filename": "/GameData/textures/FPSTextures/Flat69.png", "start": 128435193, "end": 128440687}, {"filename": "/GameData/textures/FPSTextures/Flat70.png", "start": 128440687, "end": 128446263}, {"filename": "/GameData/textures/FPSTextures/Flat71.png", "start": 128446263, "end": 128448127}, {"filename": "/GameData/textures/FPSTextures/Flat72.png", "start": 128448127, "end": 128451172}, {"filename": "/GameData/textures/FPSTextures/Flat73.png", "start": 128451172, "end": 128453895}, {"filename": "/GameData/textures/FPSTextures/Flat74.png", "start": 128453895, "end": 128456528}, {"filename": "/GameData/textures/FPSTextures/Flat75.png", "start": 128456528, "end": 128459404}, {"filename": "/GameData/textures/FPSTextures/Flat76.png", "start": 128459404, "end": 128461449}, {"filename": "/GameData/textures/FPSTextures/Flat77.png", "start": 128461449, "end": 128463639}, {"filename": "/GameData/textures/FPSTextures/Flat78.png", "start": 128463639, "end": 128466988}, {"filename": "/GameData/textures/FPSTextures/Flat79.png", "start": 128466988, "end": 128469509}, {"filename": "/GameData/textures/FPSTextures/Flat80.png", "start": 128469509, "end": 128471330}, {"filename": "/GameData/textures/FPSTextures/Flat81.png", "start": 128471330, "end": 128473918}, {"filename": "/GameData/textures/FPSTextures/Flat82.png", "start": 128473918, "end": 128476285}, {"filename": "/GameData/textures/FPSTextures/Flat83.png", "start": 128476285, "end": 128478283}, {"filename": "/GameData/textures/FPSTextures/Flat84.png", "start": 128478283, "end": 128480499}, {"filename": "/GameData/textures/FPSTextures/Flat85.png", "start": 128480499, "end": 128482593}, {"filename": "/GameData/textures/FPSTextures/Flat86.png", "start": 128482593, "end": 128484846}, {"filename": "/GameData/textures/FPSTextures/Flat87.png", "start": 128484846, "end": 128487631}, {"filename": "/GameData/textures/FPSTextures/Flat88.png", "start": 128487631, "end": 128490564}, {"filename": "/GameData/textures/FPSTextures/Flat89.png", "start": 128490564, "end": 128493422}, {"filename": "/GameData/textures/FPSTextures/Flat90.png", "start": 128493422, "end": 128496442}, {"filename": "/GameData/textures/FPSTextures/Flat91.png", "start": 128496442, "end": 128498892}, {"filename": "/GameData/textures/FPSTextures/Flat92.png", "start": 128498892, "end": 128505325}, {"filename": "/GameData/textures/FPSTextures/Flat93.png", "start": 128505325, "end": 128511331}, {"filename": "/GameData/textures/FPSTextures/Flat94.png", "start": 128511331, "end": 128519137}, {"filename": "/GameData/textures/FPSTextures/Flat95.png", "start": 128519137, "end": 128525734}, {"filename": "/GameData/textures/FPSTextures/Flat96.png", "start": 128525734, "end": 128531536}, {"filename": "/GameData/textures/FPSTextures/Flat97.png", "start": 128531536, "end": 128537725}, {"filename": "/GameData/textures/FPSTextures/Flat98.png", "start": 128537725, "end": 128543717}, {"filename": "/GameData/textures/FPSTextures/Flat99.png", "start": 128543717, "end": 128545979}, {"filename": "/GameData/textures/FPSTextures/Flat_143.png", "start": 128545979, "end": 128548546}, {"filename": "/GameData/textures/FPSTextures/Flat_144.png", "start": 128548546, "end": 128551399}, {"filename": "/GameData/textures/FPSTextures/Flat_145.png", "start": 128551399, "end": 128554130}, {"filename": "/GameData/textures/FPSTextures/Flat_146.png", "start": 128554130, "end": 128555797}, {"filename": "/GameData/textures/FPSTextures/Flat_147.png", "start": 128555797, "end": 128558475}, {"filename": "/GameData/textures/FPSTextures/Flat_148.png", "start": 128558475, "end": 128560971}, {"filename": "/GameData/textures/FPSTextures/Flat_149.png", "start": 128560971, "end": 128563330}, {"filename": "/GameData/textures/FPSTextures/Flat_150.png", "start": 128563330, "end": 128565597}, {"filename": "/GameData/textures/FPSTextures/Flat_151.png", "start": 128565597, "end": 128568266}, {"filename": "/GameData/textures/FPSTextures/Flat_152.png", "start": 128568266, "end": 128570774}, {"filename": "/GameData/textures/FPSTextures/Flat_153.png", "start": 128570774, "end": 128573783}, {"filename": "/GameData/textures/FPSTextures/Flat_154.png", "start": 128573783, "end": 128575835}, {"filename": "/GameData/textures/FPSTextures/Flat_155.png", "start": 128575835, "end": 128578733}, {"filename": "/GameData/textures/FPSTextures/Flat_156.png", "start": 128578733, "end": 128581574}, {"filename": "/GameData/textures/FPSTextures/Flat_157.png", "start": 128581574, "end": 128584708}, {"filename": "/GameData/textures/FPSTextures/Flat_158.png", "start": 128584708, "end": 128587852}, {"filename": "/GameData/textures/FPSTextures/Flat_159.png", "start": 128587852, "end": 128590094}, {"filename": "/GameData/textures/FPSTextures/Flat_160.png", "start": 128590094, "end": 128592543}, {"filename": "/GameData/textures/FPSTextures/Flat_161.png", "start": 128592543, "end": 128595893}, {"filename": "/GameData/textures/FPSTextures/Flat_162.png", "start": 128595893, "end": 128598266}, {"filename": "/GameData/textures/FPSTextures/Flat_163.png", "start": 128598266, "end": 128601357}, {"filename": "/GameData/textures/FPSTextures/Flat_164.png", "start": 128601357, "end": 128603509}, {"filename": "/GameData/textures/FPSTextures/Flat_165.png", "start": 128603509, "end": 128605669}, {"filename": "/GameData/textures/FPSTextures/Flat_166.png", "start": 128605669, "end": 128608321}, {"filename": "/GameData/textures/FPSTextures/Flat_167.png", "start": 128608321, "end": 128610594}, {"filename": "/GameData/textures/FPSTextures/Flat_168.png", "start": 128610594, "end": 128613012}, {"filename": "/GameData/textures/FPSTextures/Flat_169.png", "start": 128613012, "end": 128615769}, {"filename": "/GameData/textures/FPSTextures/Flat_170.png", "start": 128615769, "end": 128618594}, {"filename": "/GameData/textures/FPSTextures/Flat_171.png", "start": 128618594, "end": 128621425}, {"filename": "/GameData/textures/FPSTextures/Flat_173.png", "start": 128621425, "end": 128623455}, {"filename": "/GameData/textures/FPSTextures/Flat_175.png", "start": 128623455, "end": 128625538}, {"filename": "/GameData/textures/FPSTextures/Flat_176.png", "start": 128625538, "end": 128627624}, {"filename": "/GameData/textures/FPSTextures/Flat_177.png", "start": 128627624, "end": 128628822}, {"filename": "/GameData/textures/FPSTextures/Flat_178.png", "start": 128628822, "end": 128630336}, {"filename": "/GameData/textures/FPSTextures/Flat_179.png", "start": 128630336, "end": 128632819}, {"filename": "/GameData/textures/FPSTextures/Flat_180.png", "start": 128632819, "end": 128635488}, {"filename": "/GameData/textures/FPSTextures/Flat_181.png", "start": 128635488, "end": 128637280}, {"filename": "/GameData/textures/FPSTextures/Flat_182.png", "start": 128637280, "end": 128639480}, {"filename": "/GameData/textures/FPSTextures/Flat_183.png", "start": 128639480, "end": 128642023}, {"filename": "/GameData/textures/FPSTextures/Flat_184.png", "start": 128642023, "end": 128644613}, {"filename": "/GameData/textures/FPSTextures/Flat_185.png", "start": 128644613, "end": 128647503}, {"filename": "/GameData/textures/FPSTextures/Flat_186.png", "start": 128647503, "end": 128651348}, {"filename": "/GameData/textures/FPSTextures/Flat_187.png", "start": 128651348, "end": 128653299}, {"filename": "/GameData/textures/FPSTextures/Flat_188.png", "start": 128653299, "end": 128655801}, {"filename": "/GameData/textures/FPSTextures/Flat_189.png", "start": 128655801, "end": 128658137}, {"filename": "/GameData/textures/FPSTextures/Flat_190.png", "start": 128658137, "end": 128660185}, {"filename": "/GameData/textures/FPSTextures/Flat_191.png", "start": 128660185, "end": 128662865}, {"filename": "/GameData/textures/FPSTextures/Flat_192.png", "start": 128662865, "end": 128665481}, {"filename": "/GameData/textures/FPSTextures/Flat_193.png", "start": 128665481, "end": 128667290}, {"filename": "/GameData/textures/FPSTextures/License.txt", "start": 128667290, "end": 128683021}, {"filename": "/GameData/textures/FPSTextures/Light01.png", "start": 128683021, "end": 128688081}, {"filename": "/GameData/textures/FPSTextures/Light02.png", "start": 128688081, "end": 128693428}, {"filename": "/GameData/textures/FPSTextures/Light03.png", "start": 128693428, "end": 128699332}, {"filename": "/GameData/textures/FPSTextures/Light04.png", "start": 128699332, "end": 128705150}, {"filename": "/GameData/textures/FPSTextures/Light05.png", "start": 128705150, "end": 128707448}, {"filename": "/GameData/textures/FPSTextures/Light06.png", "start": 128707448, "end": 128710446}, {"filename": "/GameData/textures/FPSTextures/Light07.png", "start": 128710446, "end": 128713426}, {"filename": "/GameData/textures/FPSTextures/Light08.png", "start": 128713426, "end": 128716494}, {"filename": "/GameData/textures/FPSTextures/Light09.png", "start": 128716494, "end": 128719240}, {"filename": "/GameData/textures/FPSTextures/Light10.png", "start": 128719240, "end": 128720586}, {"filename": "/GameData/textures/FPSTextures/Metal_01.png", "start": 128720586, "end": 128727388}, {"filename": "/GameData/textures/FPSTextures/Metal_02.png", "start": 128727388, "end": 128735309}, {"filename": "/GameData/textures/FPSTextures/Step01.png", "start": 128735309, "end": 128736502}, {"filename": "/GameData/textures/FPSTextures/Step02.png", "start": 128736502, "end": 128739243}, {"filename": "/GameData/textures/FPSTextures/Step03.png", "start": 128739243, "end": 128742003}, {"filename": "/GameData/textures/FPSTextures/Step04.png", "start": 128742003, "end": 128744764}, {"filename": "/GameData/textures/FPSTextures/Step05.png", "start": 128744764, "end": 128747513}, {"filename": "/GameData/textures/FPSTextures/Step06.png", "start": 128747513, "end": 128748548}, {"filename": "/GameData/textures/FPSTextures/Step07.png", "start": 128748548, "end": 128749765}, {"filename": "/GameData/textures/FPSTextures/Step08.png", "start": 128749765, "end": 128750903}, {"filename": "/GameData/textures/FPSTextures/Strip01.png", "start": 128750903, "end": 128754236}, {"filename": "/GameData/textures/FPSTextures/Strip02.png", "start": 128754236, "end": 128757725}, {"filename": "/GameData/textures/FPSTextures/Strip03.png", "start": 128757725, "end": 128760269}, {"filename": "/GameData/textures/FPSTextures/Strip04.png", "start": 128760269, "end": 128761595}, {"filename": "/GameData/textures/FPSTextures/Strip05.png", "start": 128761595, "end": 128765680}, {"filename": "/GameData/textures/FPSTextures/Strip06.png", "start": 128765680, "end": 128770320}, {"filename": "/GameData/textures/FPSTextures/Strip07.png", "start": 128770320, "end": 128771856}, {"filename": "/GameData/textures/FPSTextures/Wall_01.png", "start": 128771856, "end": 128776758}, {"filename": "/GameData/textures/FPSTextures/Wall_02.png", "start": 128776758, "end": 128784126}, {"filename": "/GameData/textures/FPSTextures/Wall_03.png", "start": 128784126, "end": 128787844}, {"filename": "/GameData/textures/FPSTextures/Wall_04.png", "start": 128787844, "end": 128793445}, {"filename": "/GameData/textures/FPSTextures/Wall_05.png", "start": 128793445, "end": 128797421}, {"filename": "/GameData/textures/FPSTextures/Wall_06.png", "start": 128797421, "end": 128800943}, {"filename": "/GameData/textures/FPSTextures/Wall_07.png", "start": 128800943, "end": 128805927}, {"filename": "/GameData/textures/FPSTextures/Wall_08.png", "start": 128805927, "end": 128815609}, {"filename": "/GameData/textures/FPSTextures/Wall_09.png", "start": 128815609, "end": 128820996}, {"filename": "/GameData/textures/FPSTextures/Wall_10.png", "start": 128820996, "end": 128827405}, {"filename": "/GameData/textures/FPSTextures/Wall_100.png", "start": 128827405, "end": 128830326}, {"filename": "/GameData/textures/FPSTextures/Wall_101.png", "start": 128830326, "end": 128833830}, {"filename": "/GameData/textures/FPSTextures/Wall_102.png", "start": 128833830, "end": 128838303}, {"filename": "/GameData/textures/FPSTextures/Wall_103.png", "start": 128838303, "end": 128840539}, {"filename": "/GameData/textures/FPSTextures/Wall_104.png", "start": 128840539, "end": 128842320}, {"filename": "/GameData/textures/FPSTextures/Wall_105.png", "start": 128842320, "end": 128847203}, {"filename": "/GameData/textures/FPSTextures/Wall_106.png", "start": 128847203, "end": 128850564}, {"filename": "/GameData/textures/FPSTextures/Wall_107.png", "start": 128850564, "end": 128854945}, {"filename": "/GameData/textures/FPSTextures/Wall_108.png", "start": 128854945, "end": 128860427}, {"filename": "/GameData/textures/FPSTextures/Wall_109.png", "start": 128860427, "end": 128864003}, {"filename": "/GameData/textures/FPSTextures/Wall_11.png", "start": 128864003, "end": 128867263}, {"filename": "/GameData/textures/FPSTextures/Wall_110.png", "start": 128867263, "end": 128871956}, {"filename": "/GameData/textures/FPSTextures/Wall_111.png", "start": 128871956, "end": 128882537}, {"filename": "/GameData/textures/FPSTextures/Wall_112.png", "start": 128882537, "end": 128886220}, {"filename": "/GameData/textures/FPSTextures/Wall_113.png", "start": 128886220, "end": 128889480}, {"filename": "/GameData/textures/FPSTextures/Wall_114.png", "start": 128889480, "end": 128891887}, {"filename": "/GameData/textures/FPSTextures/Wall_115.png", "start": 128891887, "end": 128894491}, {"filename": "/GameData/textures/FPSTextures/Wall_116.png", "start": 128894491, "end": 128897008}, {"filename": "/GameData/textures/FPSTextures/Wall_117.png", "start": 128897008, "end": 128902922}, {"filename": "/GameData/textures/FPSTextures/Wall_118.png", "start": 128902922, "end": 128906962}, {"filename": "/GameData/textures/FPSTextures/Wall_119.png", "start": 128906962, "end": 128917174}, {"filename": "/GameData/textures/FPSTextures/Wall_12.png", "start": 128917174, "end": 128920587}, {"filename": "/GameData/textures/FPSTextures/Wall_120.png", "start": 128920587, "end": 128923054}, {"filename": "/GameData/textures/FPSTextures/Wall_121.png", "start": 128923054, "end": 128926290}, {"filename": "/GameData/textures/FPSTextures/Wall_122.png", "start": 128926290, "end": 128929886}, {"filename": "/GameData/textures/FPSTextures/Wall_123.png", "start": 128929886, "end": 128932730}, {"filename": "/GameData/textures/FPSTextures/Wall_124.png", "start": 128932730, "end": 128934927}, {"filename": "/GameData/textures/FPSTextures/Wall_125.png", "start": 128934927, "end": 128940273}, {"filename": "/GameData/textures/FPSTextures/Wall_126.png", "start": 128940273, "end": 128942944}, {"filename": "/GameData/textures/FPSTextures/Wall_127.png", "start": 128942944, "end": 128945648}, {"filename": "/GameData/textures/FPSTextures/Wall_128.png", "start": 128945648, "end": 128948270}, {"filename": "/GameData/textures/FPSTextures/Wall_129.png", "start": 128948270, "end": 128951847}, {"filename": "/GameData/textures/FPSTextures/Wall_13.png", "start": 128951847, "end": 128955204}, {"filename": "/GameData/textures/FPSTextures/Wall_130.png", "start": 128955204, "end": 128957985}, {"filename": "/GameData/textures/FPSTextures/Wall_131.png", "start": 128957985, "end": 128959559}, {"filename": "/GameData/textures/FPSTextures/Wall_132.png", "start": 128959559, "end": 128964441}, {"filename": "/GameData/textures/FPSTextures/Wall_133.png", "start": 128964441, "end": 128968859}, {"filename": "/GameData/textures/FPSTextures/Wall_134.png", "start": 128968859, "end": 128974432}, {"filename": "/GameData/textures/FPSTextures/Wall_135.png", "start": 128974432, "end": 128978147}, {"filename": "/GameData/textures/FPSTextures/Wall_136.png", "start": 128978147, "end": 128982915}, {"filename": "/GameData/textures/FPSTextures/Wall_137.png", "start": 128982915, "end": 128987532}, {"filename": "/GameData/textures/FPSTextures/Wall_138.png", "start": 128987532, "end": 128992145}, {"filename": "/GameData/textures/FPSTextures/Wall_139.png", "start": 128992145, "end": 128996059}, {"filename": "/GameData/textures/FPSTextures/Wall_14.png", "start": 128996059, "end": 128999899}, {"filename": "/GameData/textures/FPSTextures/Wall_140.png", "start": 128999899, "end": 129003859}, {"filename": "/GameData/textures/FPSTextures/Wall_141.png", "start": 129003859, "end": 129007158}, {"filename": "/GameData/textures/FPSTextures/Wall_142.png", "start": 129007158, "end": 129009869}, {"filename": "/GameData/textures/FPSTextures/Wall_143.png", "start": 129009869, "end": 129014363}, {"filename": "/GameData/textures/FPSTextures/Wall_144.png", "start": 129014363, "end": 129016585}, {"filename": "/GameData/textures/FPSTextures/Wall_145.png", "start": 129016585, "end": 129019486}, {"filename": "/GameData/textures/FPSTextures/Wall_146.png", "start": 129019486, "end": 129022231}, {"filename": "/GameData/textures/FPSTextures/Wall_147.png", "start": 129022231, "end": 129025581}, {"filename": "/GameData/textures/FPSTextures/Wall_148.png", "start": 129025581, "end": 129029002}, {"filename": "/GameData/textures/FPSTextures/Wall_149.png", "start": 129029002, "end": 129032272}, {"filename": "/GameData/textures/FPSTextures/Wall_15.png", "start": 129032272, "end": 129037402}, {"filename": "/GameData/textures/FPSTextures/Wall_150.png", "start": 129037402, "end": 129042673}, {"filename": "/GameData/textures/FPSTextures/Wall_151.png", "start": 129042673, "end": 129046880}, {"filename": "/GameData/textures/FPSTextures/Wall_152.png", "start": 129046880, "end": 129051440}, {"filename": "/GameData/textures/FPSTextures/Wall_153.png", "start": 129051440, "end": 129055716}, {"filename": "/GameData/textures/FPSTextures/Wall_154.png", "start": 129055716, "end": 129058161}, {"filename": "/GameData/textures/FPSTextures/Wall_155.png", "start": 129058161, "end": 129060421}, {"filename": "/GameData/textures/FPSTextures/Wall_156.png", "start": 129060421, "end": 129064122}, {"filename": "/GameData/textures/FPSTextures/Wall_157.png", "start": 129064122, "end": 129066989}, {"filename": "/GameData/textures/FPSTextures/Wall_158.png", "start": 129066989, "end": 129070359}, {"filename": "/GameData/textures/FPSTextures/Wall_159.png", "start": 129070359, "end": 129072450}, {"filename": "/GameData/textures/FPSTextures/Wall_16.png", "start": 129072450, "end": 129076015}, {"filename": "/GameData/textures/FPSTextures/Wall_160.png", "start": 129076015, "end": 129078750}, {"filename": "/GameData/textures/FPSTextures/Wall_161.png", "start": 129078750, "end": 129081835}, {"filename": "/GameData/textures/FPSTextures/Wall_162.png", "start": 129081835, "end": 129085908}, {"filename": "/GameData/textures/FPSTextures/Wall_163.png", "start": 129085908, "end": 129089895}, {"filename": "/GameData/textures/FPSTextures/Wall_164.png", "start": 129089895, "end": 129092656}, {"filename": "/GameData/textures/FPSTextures/Wall_166.png", "start": 129092656, "end": 129095248}, {"filename": "/GameData/textures/FPSTextures/Wall_167.png", "start": 129095248, "end": 129099786}, {"filename": "/GameData/textures/FPSTextures/Wall_168.png", "start": 129099786, "end": 129102798}, {"filename": "/GameData/textures/FPSTextures/Wall_169.png", "start": 129102798, "end": 129105234}, {"filename": "/GameData/textures/FPSTextures/Wall_17.png", "start": 129105234, "end": 129112333}, {"filename": "/GameData/textures/FPSTextures/Wall_170.png", "start": 129112333, "end": 129115247}, {"filename": "/GameData/textures/FPSTextures/Wall_171.png", "start": 129115247, "end": 129119612}, {"filename": "/GameData/textures/FPSTextures/Wall_172.png", "start": 129119612, "end": 129121825}, {"filename": "/GameData/textures/FPSTextures/Wall_173.png", "start": 129121825, "end": 129125772}, {"filename": "/GameData/textures/FPSTextures/Wall_174.png", "start": 129125772, "end": 129129652}, {"filename": "/GameData/textures/FPSTextures/Wall_175.png", "start": 129129652, "end": 129134237}, {"filename": "/GameData/textures/FPSTextures/Wall_178.png", "start": 129134237, "end": 129145474}, {"filename": "/GameData/textures/FPSTextures/Wall_179.png", "start": 129145474, "end": 129152541}, {"filename": "/GameData/textures/FPSTextures/Wall_18.png", "start": 129152541, "end": 129159451}, {"filename": "/GameData/textures/FPSTextures/Wall_180.png", "start": 129159451, "end": 129163623}, {"filename": "/GameData/textures/FPSTextures/Wall_181.png", "start": 129163623, "end": 129167946}, {"filename": "/GameData/textures/FPSTextures/Wall_182.png", "start": 129167946, "end": 129171298}, {"filename": "/GameData/textures/FPSTextures/Wall_183.png", "start": 129171298, "end": 129174433}, {"filename": "/GameData/textures/FPSTextures/Wall_184.png", "start": 129174433, "end": 129177813}, {"filename": "/GameData/textures/FPSTextures/Wall_185.png", "start": 129177813, "end": 129180760}, {"filename": "/GameData/textures/FPSTextures/Wall_186.png", "start": 129180760, "end": 129184095}, {"filename": "/GameData/textures/FPSTextures/Wall_187.png", "start": 129184095, "end": 129187488}, {"filename": "/GameData/textures/FPSTextures/Wall_188.png", "start": 129187488, "end": 129190280}, {"filename": "/GameData/textures/FPSTextures/Wall_19.png", "start": 129190280, "end": 129197879}, {"filename": "/GameData/textures/FPSTextures/Wall_20.png", "start": 129197879, "end": 129204473}, {"filename": "/GameData/textures/FPSTextures/Wall_21.png", "start": 129204473, "end": 129208968}, {"filename": "/GameData/textures/FPSTextures/Wall_22.png", "start": 129208968, "end": 129214822}, {"filename": "/GameData/textures/FPSTextures/Wall_23.png", "start": 129214822, "end": 129217840}, {"filename": "/GameData/textures/FPSTextures/Wall_24.png", "start": 129217840, "end": 129224846}, {"filename": "/GameData/textures/FPSTextures/Wall_25.png", "start": 129224846, "end": 129229674}, {"filename": "/GameData/textures/FPSTextures/Wall_26.png", "start": 129229674, "end": 129233864}, {"filename": "/GameData/textures/FPSTextures/Wall_30.png", "start": 129233864, "end": 129237626}, {"filename": "/GameData/textures/FPSTextures/Wall_31.png", "start": 129237626, "end": 129241085}, {"filename": "/GameData/textures/FPSTextures/Wall_32.png", "start": 129241085, "end": 129244506}, {"filename": "/GameData/textures/FPSTextures/Wall_33.png", "start": 129244506, "end": 129253038}, {"filename": "/GameData/textures/FPSTextures/Wall_34.png", "start": 129253038, "end": 129257982}, {"filename": "/GameData/textures/FPSTextures/Wall_35.png", "start": 129257982, "end": 129264241}, {"filename": "/GameData/textures/FPSTextures/Wall_36.png", "start": 129264241, "end": 129268987}, {"filename": "/GameData/textures/FPSTextures/Wall_37.png", "start": 129268987, "end": 129275925}, {"filename": "/GameData/textures/FPSTextures/Wall_38.png", "start": 129275925, "end": 129284740}, {"filename": "/GameData/textures/FPSTextures/Wall_39.png", "start": 129284740, "end": 129291366}, {"filename": "/GameData/textures/FPSTextures/Wall_40.png", "start": 129291366, "end": 129293056}, {"filename": "/GameData/textures/FPSTextures/Wall_41.png", "start": 129293056, "end": 129300535}, {"filename": "/GameData/textures/FPSTextures/Wall_42.png", "start": 129300535, "end": 129308148}, {"filename": "/GameData/textures/FPSTextures/Wall_43.png", "start": 129308148, "end": 129315114}, {"filename": "/GameData/textures/FPSTextures/Wall_44.png", "start": 129315114, "end": 129318155}, {"filename": "/GameData/textures/FPSTextures/Wall_45.png", "start": 129318155, "end": 129321806}, {"filename": "/GameData/textures/FPSTextures/Wall_46.png", "start": 129321806, "end": 129330532}, {"filename": "/GameData/textures/FPSTextures/Wall_47.png", "start": 129330532, "end": 129338166}, {"filename": "/GameData/textures/FPSTextures/Wall_48.png", "start": 129338166, "end": 129341544}, {"filename": "/GameData/textures/FPSTextures/Wall_49.png", "start": 129341544, "end": 129349247}, {"filename": "/GameData/textures/FPSTextures/Wall_50.png", "start": 129349247, "end": 129352624}, {"filename": "/GameData/textures/FPSTextures/Wall_51.png", "start": 129352624, "end": 129356425}, {"filename": "/GameData/textures/FPSTextures/Wall_52.png", "start": 129356425, "end": 129363930}, {"filename": "/GameData/textures/FPSTextures/Wall_53.png", "start": 129363930, "end": 129367736}, {"filename": "/GameData/textures/FPSTextures/Wall_54.png", "start": 129367736, "end": 129370725}, {"filename": "/GameData/textures/FPSTextures/Wall_55.png", "start": 129370725, "end": 129380689}, {"filename": "/GameData/textures/FPSTextures/Wall_56.png", "start": 129380689, "end": 129388007}, {"filename": "/GameData/textures/FPSTextures/Wall_57.png", "start": 129388007, "end": 129392360}, {"filename": "/GameData/textures/FPSTextures/Wall_58.png", "start": 129392360, "end": 129397007}, {"filename": "/GameData/textures/FPSTextures/Wall_59.png", "start": 129397007, "end": 129400103}, {"filename": "/GameData/textures/FPSTextures/Wall_61.png", "start": 129400103, "end": 129403281}, {"filename": "/GameData/textures/FPSTextures/Wall_62.png", "start": 129403281, "end": 129406398}, {"filename": "/GameData/textures/FPSTextures/Wall_63.png", "start": 129406398, "end": 129410518}, {"filename": "/GameData/textures/FPSTextures/Wall_64.png", "start": 129410518, "end": 129417986}, {"filename": "/GameData/textures/FPSTextures/Wall_65.png", "start": 129417986, "end": 129425932}, {"filename": "/GameData/textures/FPSTextures/Wall_66.png", "start": 129425932, "end": 129432779}, {"filename": "/GameData/textures/FPSTextures/Wall_67.png", "start": 129432779, "end": 129439706}, {"filename": "/GameData/textures/FPSTextures/Wall_68.png", "start": 129439706, "end": 129447044}, {"filename": "/GameData/textures/FPSTextures/Wall_69.png", "start": 129447044, "end": 129453290}, {"filename": "/GameData/textures/FPSTextures/Wall_70.png", "start": 129453290, "end": 129459716}, {"filename": "/GameData/textures/FPSTextures/Wall_71.png", "start": 129459716, "end": 129466779}, {"filename": "/GameData/textures/FPSTextures/Wall_72.png", "start": 129466779, "end": 129474277}, {"filename": "/GameData/textures/FPSTextures/Wall_73.png", "start": 129474277, "end": 129479485}, {"filename": "/GameData/textures/FPSTextures/Wall_74.png", "start": 129479485, "end": 129481625}, {"filename": "/GameData/textures/FPSTextures/Wall_75.png", "start": 129481625, "end": 129485380}, {"filename": "/GameData/textures/FPSTextures/Wall_76.png", "start": 129485380, "end": 129489394}, {"filename": "/GameData/textures/FPSTextures/Wall_77.png", "start": 129489394, "end": 129498130}, {"filename": "/GameData/textures/FPSTextures/Wall_78.png", "start": 129498130, "end": 129506921}, {"filename": "/GameData/textures/FPSTextures/Wall_79.png", "start": 129506921, "end": 129508982}, {"filename": "/GameData/textures/FPSTextures/Wall_80.png", "start": 129508982, "end": 129511261}, {"filename": "/GameData/textures/FPSTextures/Wall_81.png", "start": 129511261, "end": 129513721}, {"filename": "/GameData/textures/FPSTextures/Wall_82.png", "start": 129513721, "end": 129515541}, {"filename": "/GameData/textures/FPSTextures/Wall_83.png", "start": 129515541, "end": 129524273}, {"filename": "/GameData/textures/FPSTextures/Wall_84.png", "start": 129524273, "end": 129527799}, {"filename": "/GameData/textures/FPSTextures/Wall_85.png", "start": 129527799, "end": 129529563}, {"filename": "/GameData/textures/FPSTextures/Wall_86.png", "start": 129529563, "end": 129532436}, {"filename": "/GameData/textures/FPSTextures/Wall_87.png", "start": 129532436, "end": 129539446}, {"filename": "/GameData/textures/FPSTextures/Wall_88.png", "start": 129539446, "end": 129545980}, {"filename": "/GameData/textures/FPSTextures/Wall_89.png", "start": 129545980, "end": 129549724}, {"filename": "/GameData/textures/FPSTextures/Wall_90.png", "start": 129549724, "end": 129553802}, {"filename": "/GameData/textures/FPSTextures/Wall_91.png", "start": 129553802, "end": 129557797}, {"filename": "/GameData/textures/FPSTextures/Wall_92.png", "start": 129557797, "end": 129562010}, {"filename": "/GameData/textures/FPSTextures/Wall_93.png", "start": 129562010, "end": 129566299}, {"filename": "/GameData/textures/FPSTextures/Wall_94.png", "start": 129566299, "end": 129569548}, {"filename": "/GameData/textures/FPSTextures/Wall_95.png", "start": 129569548, "end": 129574456}, {"filename": "/GameData/textures/FPSTextures/Wall_96.png", "start": 129574456, "end": 129582788}, {"filename": "/GameData/textures/FPSTextures/Wall_97.png", "start": 129582788, "end": 129591550}, {"filename": "/GameData/textures/FPSTextures/Wall_98.png", "start": 129591550, "end": 129602162}, {"filename": "/GameData/textures/FPSTextures/Wall_99.png", "start": 129602162, "end": 129609233}, {"filename": "/GameData/textures/FPSTextures/flat_172.png", "start": 129609233, "end": 129611394}, {"filename": "/GameData/textures/FPSTextures/wall_165.png", "start": 129611394, "end": 129614489}, {"filename": "/GameData/textures/FPSTextures_HD/wall_wood_wallpaper.png", "start": 129614489, "end": 129660653}, {"filename": "/GameData/textures/Ground/grass.png", "start": 129660653, "end": 129664437}, {"filename": "/GameData/textures/M_Shotgun_Base_Color.png", "start": 129664437, "end": 130039579}, {"filename": "/GameData/textures/arms.png", "start": 130039579, "end": 130075434}, {"filename": "/GameData/textures/building_1/b1_f1.png", "start": 130075434, "end": 130173458}, {"filename": "/GameData/textures/building_1/b1_f2.png", "start": 130173458, "end": 130248400}, {"filename": "/GameData/textures/building_1/b1_f3.png", "start": 130248400, "end": 130325337}, {"filename": "/GameData/textures/common/trigger.png", "start": 130325337, "end": 130337213}, {"filename": "/GameData/textures/delvenPack/dlv_door1a.png", "start": 130337213, "end": 130344997}, {"filename": "/GameData/textures/delvenPack/dlv_door1b.png", "start": 130344997, "end": 130354187}, {"filename": "/GameData/textures/delvenPack/dlv_door1c.png", "start": 130354187, "end": 130362038}, {"filename": "/GameData/textures/delvenPack/dlv_door1d.png", "start": 130362038, "end": 130371210}, {"filename": "/GameData/textures/delvenPack/dlv_door2a.png", "start": 130371210, "end": 130378934}, {"filename": "/GameData/textures/delvenPack/dlv_door2b.png", "start": 130378934, "end": 130388059}, {"filename": "/GameData/textures/delvenPack/dlv_door2c.png", "start": 130388059, "end": 130396087}, {"filename": "/GameData/textures/delvenPack/dlv_door2d.png", "start": 130396087, "end": 130405315}, {"filename": "/GameData/textures/delvenPack/dlv_door3a.png", "start": 130405315, "end": 130409463}, {"filename": "/GameData/textures/delvenPack/dlv_door3b.png", "start": 130409463, "end": 130414241}, {"filename": "/GameData/textures/delvenPack/dlv_door3c.png", "start": 130414241, "end": 130418391}, {"filename": "/GameData/textures/delvenPack/dlv_door3d.png", "start": 130418391, "end": 130423183}, {"filename": "/GameData/textures/delvenPack/dlv_door4a.png", "start": 130423183, "end": 130427304}, {"filename": "/GameData/textures/delvenPack/dlv_door4b.png", "start": 130427304, "end": 130432095}, {"filename": "/GameData/textures/delvenPack/dlv_door4c.png", "start": 130432095, "end": 130436365}, {"filename": "/GameData/textures/delvenPack/dlv_door4d.png", "start": 130436365, "end": 130441201}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1a.png", "start": 130441201, "end": 130446577}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1b.png", "start": 130446577, "end": 130452705}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1c.png", "start": 130452705, "end": 130458999}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1d.png", "start": 130458999, "end": 130462635}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1e.png", "start": 130462635, "end": 130466321}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2a.png", "start": 130466321, "end": 130471046}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2b.png", "start": 130471046, "end": 130476497}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2c.png", "start": 130476497, "end": 130482175}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2d.png", "start": 130482175, "end": 130485461}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2e.png", "start": 130485461, "end": 130488822}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3a.png", "start": 130488822, "end": 130493776}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3b.png", "start": 130493776, "end": 130499447}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3c.png", "start": 130499447, "end": 130505272}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3d.png", "start": 130505272, "end": 130508652}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3e.png", "start": 130508652, "end": 130512094}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4a.png", "start": 130512094, "end": 130521512}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4b.png", "start": 130521512, "end": 130531477}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4c.png", "start": 130531477, "end": 130541606}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4d.png", "start": 130541606, "end": 130547130}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4e.png", "start": 130547130, "end": 130552700}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5a.png", "start": 130552700, "end": 130561767}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5b.png", "start": 130561767, "end": 130571414}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5c.png", "start": 130571414, "end": 130581214}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5d.png", "start": 130581214, "end": 130586624}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5e.png", "start": 130586624, "end": 130592058}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6a.png", "start": 130592058, "end": 130599192}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6b.png", "start": 130599192, "end": 130606920}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6c.png", "start": 130606920, "end": 130614815}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6d.png", "start": 130614815, "end": 130619215}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6e.png", "start": 130619215, "end": 130623673}, {"filename": "/GameData/textures/delvenPack/dlv_ground1a.png", "start": 130623673, "end": 130631595}, {"filename": "/GameData/textures/delvenPack/dlv_ground1b.png", "start": 130631595, "end": 130640431}, {"filename": "/GameData/textures/delvenPack/dlv_ground1c.png", "start": 130640431, "end": 130649927}, {"filename": "/GameData/textures/delvenPack/dlv_ground1d.png", "start": 130649927, "end": 130659970}, {"filename": "/GameData/textures/delvenPack/dlv_ground2a.png", "start": 130659970, "end": 130669838}, {"filename": "/GameData/textures/delvenPack/dlv_ground2b.png", "start": 130669838, "end": 130678745}, {"filename": "/GameData/textures/delvenPack/dlv_ground2c.png", "start": 130678745, "end": 130688470}, {"filename": "/GameData/textures/delvenPack/dlv_ground2d.png", "start": 130688470, "end": 130698891}, {"filename": "/GameData/textures/delvenPack/dlv_ground3a.png", "start": 130698891, "end": 130708950}, {"filename": "/GameData/textures/delvenPack/dlv_ground3b.png", "start": 130708950, "end": 130718662}, {"filename": "/GameData/textures/delvenPack/dlv_ground3c.png", "start": 130718662, "end": 130728098}, {"filename": "/GameData/textures/delvenPack/dlv_ground4a.png", "start": 130728098, "end": 130738289}, {"filename": "/GameData/textures/delvenPack/dlv_ground4b.png", "start": 130738289, "end": 130748739}, {"filename": "/GameData/textures/delvenPack/dlv_ground4c.png", "start": 130748739, "end": 130758680}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal1a.png", "start": 130758680, "end": 130762708}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal1b.png", "start": 130762708, "end": 130767420}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal1c.png", "start": 130767420, "end": 130771501}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal2a.png", "start": 130771501, "end": 130775554}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal2b.png", "start": 130775554, "end": 130780313}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal2c.png", "start": 130780313, "end": 130784184}, {"filename": "/GameData/textures/delvenPack/dlv_litstone1a.png", "start": 130784184, "end": 130787562}, {"filename": "/GameData/textures/delvenPack/dlv_litstone1b.png", "start": 130787562, "end": 130791775}, {"filename": "/GameData/textures/delvenPack/dlv_litstone1c.png", "start": 130791775, "end": 130795406}, {"filename": "/GameData/textures/delvenPack/dlv_litstone2a.png", "start": 130795406, "end": 130800203}, {"filename": "/GameData/textures/delvenPack/dlv_litstone2b.png", "start": 130800203, "end": 130805729}, {"filename": "/GameData/textures/delvenPack/dlv_litstone2c.png", "start": 130805729, "end": 130809972}, {"filename": "/GameData/textures/delvenPack/dlv_metalgen1.png", "start": 130809972, "end": 130815797}, {"filename": "/GameData/textures/delvenPack/dlv_metalgen2.png", "start": 130815797, "end": 130822677}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan1a.png", "start": 130822677, "end": 130830735}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan1b.png", "start": 130830735, "end": 130838205}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan1c.png", "start": 130838205, "end": 130847407}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan2a.png", "start": 130847407, "end": 130855734}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan2b.png", "start": 130855734, "end": 130863146}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan2c.png", "start": 130863146, "end": 130872345}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan3a.png", "start": 130872345, "end": 130880952}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan3b.png", "start": 130880952, "end": 130889179}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan3c.png", "start": 130889179, "end": 130898481}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan4a.png", "start": 130898481, "end": 130907242}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan4b.png", "start": 130907242, "end": 130915567}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan4c.png", "start": 130915567, "end": 130925022}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip1a.png", "start": 130925022, "end": 130931666}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip1b.png", "start": 130931666, "end": 130938178}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip2a.png", "start": 130938178, "end": 130945311}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip2b.png", "start": 130945311, "end": 130952329}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip3a.png", "start": 130952329, "end": 130959600}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip3b.png", "start": 130959600, "end": 130966678}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip4a.png", "start": 130966678, "end": 130974246}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip4b.png", "start": 130974246, "end": 130981601}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm1a.png", "start": 130981601, "end": 130988599}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm1b.png", "start": 130988599, "end": 130995132}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm1c.png", "start": 130995132, "end": 131003320}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm2a.png", "start": 131003320, "end": 131010817}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm2b.png", "start": 131010817, "end": 131017405}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm2c.png", "start": 131017405, "end": 131025666}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm3a.png", "start": 131025666, "end": 131032942}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm3b.png", "start": 131032942, "end": 131039868}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm3c.png", "start": 131039868, "end": 131047863}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm4a.png", "start": 131047863, "end": 131054743}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm4b.png", "start": 131054743, "end": 131061451}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm4c.png", "start": 131061451, "end": 131069260}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm5.png", "start": 131069260, "end": 131072919}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm6.png", "start": 131072919, "end": 131076642}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1a.png", "start": 131076642, "end": 131083434}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1b.png", "start": 131083434, "end": 131091805}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1c.png", "start": 131091805, "end": 131099475}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1d.png", "start": 131099475, "end": 131107592}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1e.png", "start": 131107592, "end": 131115484}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2a.png", "start": 131115484, "end": 131123675}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2b.png", "start": 131123675, "end": 131133187}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2c.png", "start": 131133187, "end": 131142089}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2d.png", "start": 131142089, "end": 131151340}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2e.png", "start": 131151340, "end": 131160411}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3a.png", "start": 131160411, "end": 131168542}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3b.png", "start": 131168542, "end": 131178252}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3c.png", "start": 131178252, "end": 131187224}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3d.png", "start": 131187224, "end": 131196698}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3e.png", "start": 131196698, "end": 131205925}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr4a.png", "start": 131205925, "end": 131214431}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr4b.png", "start": 131214431, "end": 131222891}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr4c.png", "start": 131222891, "end": 131231555}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr5a.png", "start": 131231555, "end": 131238734}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr5b.png", "start": 131238734, "end": 131245862}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr5c.png", "start": 131245862, "end": 131254002}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr6a.png", "start": 131254002, "end": 131262678}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr6b.png", "start": 131262678, "end": 131271093}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr6c.png", "start": 131271093, "end": 131279826}, {"filename": "/GameData/textures/delvenPack/dlv_slategen1.png", "start": 131279826, "end": 131285787}, {"filename": "/GameData/textures/delvenPack/dlv_slategen2.png", "start": 131285787, "end": 131293346}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1a.png", "start": 131293346, "end": 131299296}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1b.png", "start": 131299296, "end": 131305112}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1c.png", "start": 131305112, "end": 131311056}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1d.png", "start": 131311056, "end": 131316702}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2a.png", "start": 131316702, "end": 131325574}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2b.png", "start": 131325574, "end": 131334248}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2c.png", "start": 131334248, "end": 131343045}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2d.png", "start": 131343045, "end": 131351341}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk3a.png", "start": 131351341, "end": 131359788}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk3b.png", "start": 131359788, "end": 131368435}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4a.png", "start": 131368435, "end": 131375054}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4b.png", "start": 131375054, "end": 131381740}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4c.png", "start": 131381740, "end": 131388357}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4d.png", "start": 131388357, "end": 131394971}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4e.png", "start": 131394971, "end": 131401555}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4f.png", "start": 131401555, "end": 131407912}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5a.png", "start": 131407912, "end": 131417389}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5b.png", "start": 131417389, "end": 131426901}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5c.png", "start": 131426901, "end": 131436399}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5d.png", "start": 131436399, "end": 131445872}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5e.png", "start": 131445872, "end": 131455263}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5f.png", "start": 131455263, "end": 131464400}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk6a.png", "start": 131464400, "end": 131473325}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk6b.png", "start": 131473325, "end": 131482550}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr1a.png", "start": 131482550, "end": 131487980}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr1b.png", "start": 131487980, "end": 131494402}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr1c.png", "start": 131494402, "end": 131501425}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr2a.png", "start": 131501425, "end": 131509313}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr2b.png", "start": 131509313, "end": 131518128}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr2c.png", "start": 131518128, "end": 131527409}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr3a.png", "start": 131527409, "end": 131533112}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr3b.png", "start": 131533112, "end": 131539590}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr3c.png", "start": 131539590, "end": 131546582}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr4a.png", "start": 131546582, "end": 131554974}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr4b.png", "start": 131554974, "end": 131564043}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr4c.png", "start": 131564043, "end": 131573143}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr5a.png", "start": 131573143, "end": 131579233}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr5b.png", "start": 131579233, "end": 131585646}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr5c.png", "start": 131585646, "end": 131592564}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr6a.png", "start": 131592564, "end": 131601109}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr6b.png", "start": 131601109, "end": 131609929}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr6c.png", "start": 131609929, "end": 131619213}, {"filename": "/GameData/textures/delvenPack/dlv_stonegen1.png", "start": 131619213, "end": 131624307}, {"filename": "/GameData/textures/delvenPack/dlv_stonegen2.png", "start": 131624307, "end": 131631737}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep1a.png", "start": 131631737, "end": 131637180}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep1b.png", "start": 131637180, "end": 131642720}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep2a.png", "start": 131642720, "end": 131650569}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep2b.png", "start": 131650569, "end": 131658319}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm1a.png", "start": 131658319, "end": 131663609}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm1b.png", "start": 131663609, "end": 131669709}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm1c.png", "start": 131669709, "end": 131675285}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm2a.png", "start": 131675285, "end": 131683031}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm2b.png", "start": 131683031, "end": 131691861}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm2c.png", "start": 131691861, "end": 131700169}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm3a.png", "start": 131700169, "end": 131705469}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm3b.png", "start": 131705469, "end": 131711542}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm4a.png", "start": 131711542, "end": 131719364}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm4b.png", "start": 131719364, "end": 131727895}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm5.png", "start": 131727895, "end": 131731052}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm6.png", "start": 131731052, "end": 131735407}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf1a.png", "start": 131735407, "end": 131741018}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf1b.png", "start": 131741018, "end": 131746635}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf2a.png", "start": 131746635, "end": 131754508}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf2b.png", "start": 131754508, "end": 131762617}, {"filename": "/GameData/textures/delvenPack/dlv_wood1a.png", "start": 131762617, "end": 131767316}, {"filename": "/GameData/textures/delvenPack/dlv_wood1b.png", "start": 131767316, "end": 131772275}, {"filename": "/GameData/textures/delvenPack/dlv_wood2a.png", "start": 131772275, "end": 131778971}, {"filename": "/GameData/textures/delvenPack/dlv_wood2b.png", "start": 131778971, "end": 131785928}, {"filename": "/GameData/textures/delvenPack/dlv_wood3a.png", "start": 131785928, "end": 131791132}, {"filename": "/GameData/textures/delvenPack/dlv_wood3b.png", "start": 131791132, "end": 131796590}, {"filename": "/GameData/textures/delvenPack/dlv_wood4a.png", "start": 131796590, "end": 131803422}, {"filename": "/GameData/textures/delvenPack/dlv_wood4b.png", "start": 131803422, "end": 131810467}, {"filename": "/GameData/textures/delvenPack/dlv_wood5a.png", "start": 131810467, "end": 131816480}, {"filename": "/GameData/textures/delvenPack/dlv_wood5b.png", "start": 131816480, "end": 131823315}, {"filename": "/GameData/textures/delvenPack/dlv_wood5c.png", "start": 131823315, "end": 131830325}, {"filename": "/GameData/textures/delvenPack/dlv_wood5d.png", "start": 131830325, "end": 131837915}, {"filename": "/GameData/textures/delvenPack/dlv_wood5e.png", "start": 131837915, "end": 131847048}, {"filename": "/GameData/textures/delvenPack/dlv_wood5f.png", "start": 131847048, "end": 131856077}, {"filename": "/GameData/textures/delvenPack/dlv_wood5g.png", "start": 131856077, "end": 131864811}, {"filename": "/GameData/textures/delvenPack/dlv_wood5h.png", "start": 131864811, "end": 131873539}, {"filename": "/GameData/textures/delvenPack/dlv_wood6a.png", "start": 131873539, "end": 131881169}, {"filename": "/GameData/textures/delvenPack/dlv_wood6b.png", "start": 131881169, "end": 131889617}, {"filename": "/GameData/textures/delvenPack/dlv_wood6c.png", "start": 131889617, "end": 131897185}, {"filename": "/GameData/textures/delvenPack/dlv_wood6d.png", "start": 131897185, "end": 131905582}, {"filename": "/GameData/textures/delvenPack/dlv_wood6e.png", "start": 131905582, "end": 131915339}, {"filename": "/GameData/textures/delvenPack/dlv_wood6f.png", "start": 131915339, "end": 131925285}, {"filename": "/GameData/textures/delvenPack/dlv_wood6g.png", "start": 131925285, "end": 131934488}, {"filename": "/GameData/textures/delvenPack/dlv_wood6h.png", "start": 131934488, "end": 131943927}, {"filename": "/GameData/textures/delvenPack/dlv_woodgen1.png", "start": 131943927, "end": 131948474}, {"filename": "/GameData/textures/delvenPack/dlv_woodgen2.png", "start": 131948474, "end": 131955167}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1a.png", "start": 131955167, "end": 131962954}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1b.png", "start": 131962954, "end": 131970101}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1c.png", "start": 131970101, "end": 131974371}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1d.png", "start": 131974371, "end": 131978381}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2a.png", "start": 131978381, "end": 131985520}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2b.png", "start": 131985520, "end": 131992079}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2c.png", "start": 131992079, "end": 131995900}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2d.png", "start": 131995900, "end": 131999538}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3a.png", "start": 131999538, "end": 132006667}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3b.png", "start": 132006667, "end": 132013253}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3c.png", "start": 132013253, "end": 132017226}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3d.png", "start": 132017226, "end": 132020998}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4a.png", "start": 132020998, "end": 132031880}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4b.png", "start": 132031880, "end": 132040718}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4c.png", "start": 132040718, "end": 132046220}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4d.png", "start": 132046220, "end": 132050930}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5a.png", "start": 132050930, "end": 132061514}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5b.png", "start": 132061514, "end": 132070140}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5c.png", "start": 132070140, "end": 132075449}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5d.png", "start": 132075449, "end": 132079999}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6a.png", "start": 132079999, "end": 132088848}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6b.png", "start": 132088848, "end": 132096126}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6c.png", "start": 132096126, "end": 132100744}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6d.png", "start": 132100744, "end": 132104756}, {"filename": "/GameData/textures/generic/__TB_empty.png", "start": 132104756, "end": 132105514}, {"filename": "/GameData/textures/generic/brick.png", "start": 132105514, "end": 132618428}, {"filename": "/GameData/textures/generic/brickPBR.png", "start": 132618428, "end": 133548560}, {"filename": "/GameData/textures/generic/brickPBR_orm.png", "start": 133548560, "end": 133973411}, {"filename": "/GameData/textures/generic/bricks.png", "start": 133973411, "end": 133984331}, {"filename": "/GameData/textures/generic/cat.png", "start": 133984331, "end": 134233769}, {"filename": "/GameData/textures/generic/foil.png", "start": 134233769, "end": 134490987}, {"filename": "/GameData/textures/generic/grass.png", "start": 134490987, "end": 134619112}, {"filename": "/GameData/textures/generic/hole_t.png", "start": 134619112, "end": 134621541}, {"filename": "/GameData/textures/generic/light.png", "start": 134621541, "end": 134621669}, {"filename": "/GameData/textures/generic/light_em.png", "start": 134621669, "end": 134621797}, {"filename": "/GameData/textures/generic/mask_test_m.png", "start": 134621797, "end": 134625859}, {"filename": "/GameData/textures/generic/mirror.png", "start": 134625859, "end": 134625979}, {"filename": "/GameData/textures/generic/mirror_orm.png", "start": 134625979, "end": 134626099}, {"filename": "/GameData/textures/generic/null_m.png", "start": 134626099, "end": 134627896}, {"filename": "/GameData/textures/generic/trigger_t.png", "start": 134627896, "end": 134639772}, {"filename": "/GameData/textures/generic/white.png", "start": 134639772, "end": 134639892}, {"filename": "/GameData/textures/gloves.png", "start": 134639892, "end": 134749567}, {"filename": "/GameData/textures/jacket.png", "start": 134749567, "end": 134913379}, {"filename": "/GameData/textures/lq_conc/conc1_1.png", "start": 134913379, "end": 134926492}, {"filename": "/GameData/textures/lq_conc/conc1_10.png", "start": 134926492, "end": 134940606}, {"filename": "/GameData/textures/lq_conc/conc1_2.png", "start": 134940606, "end": 134954647}, {"filename": "/GameData/textures/lq_conc/conc1_3.png", "start": 134954647, "end": 134969157}, {"filename": "/GameData/textures/lq_conc/conc1_4.png", "start": 134969157, "end": 134983543}, {"filename": "/GameData/textures/lq_conc/conc1_5.png", "start": 134983543, "end": 134997992}, {"filename": "/GameData/textures/lq_conc/conc1_6.png", "start": 134997992, "end": 135012609}, {"filename": "/GameData/textures/lq_conc/conc1_7.png", "start": 135012609, "end": 135027206}, {"filename": "/GameData/textures/lq_conc/conc1_8.png", "start": 135027206, "end": 135041554}, {"filename": "/GameData/textures/lq_conc/conc1_9.png", "start": 135041554, "end": 135055712}, {"filename": "/GameData/textures/lq_conc/conc1_a1.png", "start": 135055712, "end": 135070160}, {"filename": "/GameData/textures/lq_conc/conc2_1.png", "start": 135070160, "end": 135079588}, {"filename": "/GameData/textures/lq_conc/conc2_10.png", "start": 135079588, "end": 135089037}, {"filename": "/GameData/textures/lq_conc/conc2_2.png", "start": 135089037, "end": 135098431}, {"filename": "/GameData/textures/lq_conc/conc2_3.png", "start": 135098431, "end": 135108076}, {"filename": "/GameData/textures/lq_conc/conc2_4.png", "start": 135108076, "end": 135117583}, {"filename": "/GameData/textures/lq_conc/conc2_5.png", "start": 135117583, "end": 135127088}, {"filename": "/GameData/textures/lq_conc/conc2_6.png", "start": 135127088, "end": 135136601}, {"filename": "/GameData/textures/lq_conc/conc2_7.png", "start": 135136601, "end": 135146146}, {"filename": "/GameData/textures/lq_conc/conc2_8.png", "start": 135146146, "end": 135155556}, {"filename": "/GameData/textures/lq_conc/conc2_9.png", "start": 135155556, "end": 135165093}, {"filename": "/GameData/textures/lq_conc/conc2_a1.png", "start": 135165093, "end": 135174123}, {"filename": "/GameData/textures/lq_conc/conc3_1.png", "start": 135174123, "end": 135181987}, {"filename": "/GameData/textures/lq_conc/conc3_10.png", "start": 135181987, "end": 135190923}, {"filename": "/GameData/textures/lq_conc/conc3_2.png", "start": 135190923, "end": 135199317}, {"filename": "/GameData/textures/lq_conc/conc3_3.png", "start": 135199317, "end": 135209544}, {"filename": "/GameData/textures/lq_conc/conc3_4.png", "start": 135209544, "end": 135218823}, {"filename": "/GameData/textures/lq_conc/conc3_5.png", "start": 135218823, "end": 135228553}, {"filename": "/GameData/textures/lq_conc/conc3_6.png", "start": 135228553, "end": 135238113}, {"filename": "/GameData/textures/lq_conc/conc3_7.png", "start": 135238113, "end": 135246938}, {"filename": "/GameData/textures/lq_conc/conc3_8.png", "start": 135246938, "end": 135255719}, {"filename": "/GameData/textures/lq_conc/conc3_9.png", "start": 135255719, "end": 135265195}, {"filename": "/GameData/textures/lq_conc/conc3_a1.png", "start": 135265195, "end": 135273602}, {"filename": "/GameData/textures/lq_conc/conc4_1.png", "start": 135273602, "end": 135279761}, {"filename": "/GameData/textures/lq_conc/conc4_10.png", "start": 135279761, "end": 135286121}, {"filename": "/GameData/textures/lq_conc/conc4_2.png", "start": 135286121, "end": 135292386}, {"filename": "/GameData/textures/lq_conc/conc4_3.png", "start": 135292386, "end": 135299312}, {"filename": "/GameData/textures/lq_conc/conc4_4.png", "start": 135299312, "end": 135305802}, {"filename": "/GameData/textures/lq_conc/conc4_5.png", "start": 135305802, "end": 135312395}, {"filename": "/GameData/textures/lq_conc/conc4_6.png", "start": 135312395, "end": 135318919}, {"filename": "/GameData/textures/lq_conc/conc4_7.png", "start": 135318919, "end": 135325327}, {"filename": "/GameData/textures/lq_conc/conc4_8.png", "start": 135325327, "end": 135331610}, {"filename": "/GameData/textures/lq_conc/conc4_9.png", "start": 135331610, "end": 135338193}, {"filename": "/GameData/textures/lq_conc/conc4_a1.png", "start": 135338193, "end": 135344262}, {"filename": "/GameData/textures/lq_conc/conc5_1.png", "start": 135344262, "end": 135353024}, {"filename": "/GameData/textures/lq_conc/conc5_10.png", "start": 135353024, "end": 135362639}, {"filename": "/GameData/textures/lq_conc/conc5_2.png", "start": 135362639, "end": 135371955}, {"filename": "/GameData/textures/lq_conc/conc5_8.png", "start": 135371955, "end": 135381035}, {"filename": "/GameData/textures/lq_conc/conc5_9.png", "start": 135381035, "end": 135390749}, {"filename": "/GameData/textures/lq_conc/conc5_a1.png", "start": 135390749, "end": 135399124}, {"filename": "/GameData/textures/lq_conc/conc6_1.png", "start": 135399124, "end": 135405537}, {"filename": "/GameData/textures/lq_conc/conc6_10.png", "start": 135405537, "end": 135411946}, {"filename": "/GameData/textures/lq_conc/conc6_2.png", "start": 135411946, "end": 135418274}, {"filename": "/GameData/textures/lq_conc/conc6_3.png", "start": 135418274, "end": 135425135}, {"filename": "/GameData/textures/lq_conc/conc6_4.png", "start": 135425135, "end": 135431743}, {"filename": "/GameData/textures/lq_conc/conc6_5.png", "start": 135431743, "end": 135438419}, {"filename": "/GameData/textures/lq_conc/conc6_6.png", "start": 135438419, "end": 135445193}, {"filename": "/GameData/textures/lq_conc/conc6_7.png", "start": 135445193, "end": 135451896}, {"filename": "/GameData/textures/lq_conc/conc6_8.png", "start": 135451896, "end": 135458392}, {"filename": "/GameData/textures/lq_conc/conc6_9.png", "start": 135458392, "end": 135465014}, {"filename": "/GameData/textures/lq_conc/conc6_a1.png", "start": 135465014, "end": 135471658}, {"filename": "/GameData/textures/lq_conc/conc7_1.png", "start": 135471658, "end": 135479448}, {"filename": "/GameData/textures/lq_conc/conc7_10.png", "start": 135479448, "end": 135487126}, {"filename": "/GameData/textures/lq_conc/conc7_2.png", "start": 135487126, "end": 135494752}, {"filename": "/GameData/textures/lq_conc/conc7_3.png", "start": 135494752, "end": 135502696}, {"filename": "/GameData/textures/lq_conc/conc7_4.png", "start": 135502696, "end": 135510464}, {"filename": "/GameData/textures/lq_conc/conc7_5.png", "start": 135510464, "end": 135518288}, {"filename": "/GameData/textures/lq_conc/conc7_6.png", "start": 135518288, "end": 135526287}, {"filename": "/GameData/textures/lq_conc/conc7_7.png", "start": 135526287, "end": 135534267}, {"filename": "/GameData/textures/lq_conc/conc7_8.png", "start": 135534267, "end": 135542091}, {"filename": "/GameData/textures/lq_conc/conc7_9.png", "start": 135542091, "end": 135549891}, {"filename": "/GameData/textures/lq_conc/conc7_a1.png", "start": 135549891, "end": 135557624}, {"filename": "/GameData/textures/lq_conc/flr1_1.png", "start": 135557624, "end": 135559956}, {"filename": "/GameData/textures/lq_conc/flr1_2.png", "start": 135559956, "end": 135563220}, {"filename": "/GameData/textures/lq_conc/flr1_3.png", "start": 135563220, "end": 135566760}, {"filename": "/GameData/textures/lq_conc/flr1_4.png", "start": 135566760, "end": 135569537}, {"filename": "/GameData/textures/lq_conc/flr1_5.png", "start": 135569537, "end": 135573133}, {"filename": "/GameData/textures/lq_conc/flr1_6.png", "start": 135573133, "end": 135576189}, {"filename": "/GameData/textures/lq_conc/flr2_1.png", "start": 135576189, "end": 135578299}, {"filename": "/GameData/textures/lq_conc/flr2_2.png", "start": 135578299, "end": 135581512}, {"filename": "/GameData/textures/lq_conc/flr2_3.png", "start": 135581512, "end": 135585132}, {"filename": "/GameData/textures/lq_conc/flr2_4.png", "start": 135585132, "end": 135588530}, {"filename": "/GameData/textures/lq_conc/flr2_5.png", "start": 135588530, "end": 135591890}, {"filename": "/GameData/textures/lq_conc/flr2_6.png", "start": 135591890, "end": 135595089}, {"filename": "/GameData/textures/lq_conc/flr2_7.png", "start": 135595089, "end": 135598224}, {"filename": "/GameData/textures/lq_conc/flr2_8.png", "start": 135598224, "end": 135601165}, {"filename": "/GameData/textures/lq_dev/clip.png", "start": 135601165, "end": 135601671}, {"filename": "/GameData/textures/lq_dev/dot_blue_a.png", "start": 135601671, "end": 135602227}, {"filename": "/GameData/textures/lq_dev/dot_blue_b.png", "start": 135602227, "end": 135602782}, {"filename": "/GameData/textures/lq_dev/dot_blue_c.png", "start": 135602782, "end": 135603337}, {"filename": "/GameData/textures/lq_dev/dot_brown_a.png", "start": 135603337, "end": 135603892}, {"filename": "/GameData/textures/lq_dev/dot_brown_b.png", "start": 135603892, "end": 135604447}, {"filename": "/GameData/textures/lq_dev/dot_brown_c.png", "start": 135604447, "end": 135605002}, {"filename": "/GameData/textures/lq_dev/dot_green_a.png", "start": 135605002, "end": 135605558}, {"filename": "/GameData/textures/lq_dev/dot_green_b.png", "start": 135605558, "end": 135606113}, {"filename": "/GameData/textures/lq_dev/dot_green_c.png", "start": 135606113, "end": 135606668}, {"filename": "/GameData/textures/lq_dev/dot_grey_a.png", "start": 135606668, "end": 135607221}, {"filename": "/GameData/textures/lq_dev/dot_grey_b.png", "start": 135607221, "end": 135607777}, {"filename": "/GameData/textures/lq_dev/dot_grey_c.png", "start": 135607777, "end": 135608332}, {"filename": "/GameData/textures/lq_dev/dot_olive_a.png", "start": 135608332, "end": 135608887}, {"filename": "/GameData/textures/lq_dev/dot_olive_b.png", "start": 135608887, "end": 135609441}, {"filename": "/GameData/textures/lq_dev/dot_olive_c.png", "start": 135609441, "end": 135609996}, {"filename": "/GameData/textures/lq_dev/dot_orange_a.png", "start": 135609996, "end": 135610552}, {"filename": "/GameData/textures/lq_dev/dot_orange_b.png", "start": 135610552, "end": 135611107}, {"filename": "/GameData/textures/lq_dev/dot_orange_c.png", "start": 135611107, "end": 135611662}, {"filename": "/GameData/textures/lq_dev/dot_pink_a.png", "start": 135611662, "end": 135612218}, {"filename": "/GameData/textures/lq_dev/dot_pink_b.png", "start": 135612218, "end": 135612774}, {"filename": "/GameData/textures/lq_dev/dot_pink_c.png", "start": 135612774, "end": 135613329}, {"filename": "/GameData/textures/lq_dev/dot_purple_a.png", "start": 135613329, "end": 135613885}, {"filename": "/GameData/textures/lq_dev/dot_purple_b.png", "start": 135613885, "end": 135614440}, {"filename": "/GameData/textures/lq_dev/dot_purple_c.png", "start": 135614440, "end": 135614995}, {"filename": "/GameData/textures/lq_dev/dot_red_a.png", "start": 135614995, "end": 135615546}, {"filename": "/GameData/textures/lq_dev/dot_red_b.png", "start": 135615546, "end": 135616097}, {"filename": "/GameData/textures/lq_dev/dot_red_c.png", "start": 135616097, "end": 135616648}, {"filename": "/GameData/textures/lq_dev/dot_tan_a.png", "start": 135616648, "end": 135617204}, {"filename": "/GameData/textures/lq_dev/dot_tan_b.png", "start": 135617204, "end": 135617760}, {"filename": "/GameData/textures/lq_dev/dot_tan_c.png", "start": 135617760, "end": 135618315}, {"filename": "/GameData/textures/lq_dev/dot_yellow_a.png", "start": 135618315, "end": 135618871}, {"filename": "/GameData/textures/lq_dev/dot_yellow_b.png", "start": 135618871, "end": 135619427}, {"filename": "/GameData/textures/lq_dev/dot_yellow_c.png", "start": 135619427, "end": 135619981}, {"filename": "/GameData/textures/lq_dev/floor_blue_a.png", "start": 135619981, "end": 135620798}, {"filename": "/GameData/textures/lq_dev/floor_blue_b.png", "start": 135620798, "end": 135621616}, {"filename": "/GameData/textures/lq_dev/floor_blue_c.png", "start": 135621616, "end": 135622434}, {"filename": "/GameData/textures/lq_dev/floor_brown_a.png", "start": 135622434, "end": 135623256}, {"filename": "/GameData/textures/lq_dev/floor_brown_b.png", "start": 135623256, "end": 135624079}, {"filename": "/GameData/textures/lq_dev/floor_brown_c.png", "start": 135624079, "end": 135624901}, {"filename": "/GameData/textures/lq_dev/floor_green_a.png", "start": 135624901, "end": 135625722}, {"filename": "/GameData/textures/lq_dev/floor_green_b.png", "start": 135625722, "end": 135626544}, {"filename": "/GameData/textures/lq_dev/floor_green_c.png", "start": 135626544, "end": 135627366}, {"filename": "/GameData/textures/lq_dev/floor_grey_a.png", "start": 135627366, "end": 135628176}, {"filename": "/GameData/textures/lq_dev/floor_grey_b.png", "start": 135628176, "end": 135628989}, {"filename": "/GameData/textures/lq_dev/floor_grey_c.png", "start": 135628989, "end": 135629803}, {"filename": "/GameData/textures/lq_dev/floor_olive_a.png", "start": 135629803, "end": 135630621}, {"filename": "/GameData/textures/lq_dev/floor_olive_b.png", "start": 135630621, "end": 135631436}, {"filename": "/GameData/textures/lq_dev/floor_olive_c.png", "start": 135631436, "end": 135632253}, {"filename": "/GameData/textures/lq_dev/floor_orange_a.png", "start": 135632253, "end": 135633076}, {"filename": "/GameData/textures/lq_dev/floor_orange_b.png", "start": 135633076, "end": 135633899}, {"filename": "/GameData/textures/lq_dev/floor_orange_c.png", "start": 135633899, "end": 135634719}, {"filename": "/GameData/textures/lq_dev/floor_pink_a.png", "start": 135634719, "end": 135635542}, {"filename": "/GameData/textures/lq_dev/floor_pink_b.png", "start": 135635542, "end": 135636364}, {"filename": "/GameData/textures/lq_dev/floor_pink_c.png", "start": 135636364, "end": 135637186}, {"filename": "/GameData/textures/lq_dev/floor_purple_a.png", "start": 135637186, "end": 135638008}, {"filename": "/GameData/textures/lq_dev/floor_purple_b.png", "start": 135638008, "end": 135638830}, {"filename": "/GameData/textures/lq_dev/floor_purple_c.png", "start": 135638830, "end": 135639651}, {"filename": "/GameData/textures/lq_dev/floor_red_a.png", "start": 135639651, "end": 135640461}, {"filename": "/GameData/textures/lq_dev/floor_red_b.png", "start": 135640461, "end": 135641271}, {"filename": "/GameData/textures/lq_dev/floor_red_c.png", "start": 135641271, "end": 135642082}, {"filename": "/GameData/textures/lq_dev/floor_tan_a.png", "start": 135642082, "end": 135642902}, {"filename": "/GameData/textures/lq_dev/floor_tan_b.png", "start": 135642902, "end": 135643726}, {"filename": "/GameData/textures/lq_dev/floor_tan_c.png", "start": 135643726, "end": 135644547}, {"filename": "/GameData/textures/lq_dev/floor_yellow_a.png", "start": 135644547, "end": 135645368}, {"filename": "/GameData/textures/lq_dev/floor_yellow_b.png", "start": 135645368, "end": 135646190}, {"filename": "/GameData/textures/lq_dev/floor_yellow_c.png", "start": 135646190, "end": 135647009}, {"filename": "/GameData/textures/lq_dev/hint.png", "start": 135647009, "end": 135647875}, {"filename": "/GameData/textures/lq_dev/hintskip.png", "start": 135647875, "end": 135648768}, {"filename": "/GameData/textures/lq_dev/key_gold_1.png", "start": 135648768, "end": 135649523}, {"filename": "/GameData/textures/lq_dev/key_silver_1.png", "start": 135649523, "end": 135650277}, {"filename": "/GameData/textures/lq_dev/light_fbr.png", "start": 135650277, "end": 135651193}, {"filename": "/GameData/textures/lq_dev/origin.png", "start": 135651193, "end": 135651676}, {"filename": "/GameData/textures/lq_dev/plus_0_button_fbr.png", "start": 135651676, "end": 135651875}, {"filename": "/GameData/textures/lq_dev/plus_0_shoot_fbr.png", "start": 135651875, "end": 135652154}, {"filename": "/GameData/textures/lq_dev/plus_1_button_fbr.png", "start": 135652154, "end": 135652355}, {"filename": "/GameData/textures/lq_dev/plus_1_shoot_fbr.png", "start": 135652355, "end": 135652636}, {"filename": "/GameData/textures/lq_dev/plus_a_button_fbr.png", "start": 135652636, "end": 135652837}, {"filename": "/GameData/textures/lq_dev/plus_a_shoot_fbr.png", "start": 135652837, "end": 135653124}, {"filename": "/GameData/textures/lq_dev/skip.png", "start": 135653124, "end": 135653612}, {"filename": "/GameData/textures/lq_dev/sky_dev.png", "start": 135653612, "end": 135655584}, {"filename": "/GameData/textures/lq_dev/sky_dev.png.bak", "start": 135655584, "end": 135667938}, {"filename": "/GameData/textures/lq_dev/sky_dev_day_fbr.png", "start": 135667938, "end": 135671764}, {"filename": "/GameData/textures/lq_dev/sky_dev_void.png", "start": 135671764, "end": 135672675}, {"filename": "/GameData/textures/lq_dev/star_blood1.png", "start": 135672675, "end": 135673037}, {"filename": "/GameData/textures/lq_dev/star_lava1.png", "start": 135673037, "end": 135673408}, {"filename": "/GameData/textures/lq_dev/star_lavaskip.png", "start": 135673408, "end": 135674521}, {"filename": "/GameData/textures/lq_dev/star_slime1.png", "start": 135674521, "end": 135674891}, {"filename": "/GameData/textures/lq_dev/star_slimeskip.png", "start": 135674891, "end": 135675964}, {"filename": "/GameData/textures/lq_dev/star_smile.png", "start": 135675964, "end": 135676444}, {"filename": "/GameData/textures/lq_dev/star_teleport.png", "start": 135676444, "end": 135676768}, {"filename": "/GameData/textures/lq_dev/star_water1.png", "start": 135676768, "end": 135677140}, {"filename": "/GameData/textures/lq_dev/star_water2.png", "start": 135677140, "end": 135677509}, {"filename": "/GameData/textures/lq_dev/star_waterskip.png", "start": 135677509, "end": 135679249}, {"filename": "/GameData/textures/lq_dev/trigger.png", "start": 135679249, "end": 135679746}, {"filename": "/GameData/textures/lq_dev/wall_blue_a.png", "start": 135679746, "end": 135680426}, {"filename": "/GameData/textures/lq_dev/wall_blue_b.png", "start": 135680426, "end": 135681106}, {"filename": "/GameData/textures/lq_dev/wall_blue_c.png", "start": 135681106, "end": 135681786}, {"filename": "/GameData/textures/lq_dev/wall_brown_a.png", "start": 135681786, "end": 135682468}, {"filename": "/GameData/textures/lq_dev/wall_brown_b.png", "start": 135682468, "end": 135683150}, {"filename": "/GameData/textures/lq_dev/wall_brown_c.png", "start": 135683150, "end": 135683832}, {"filename": "/GameData/textures/lq_dev/wall_green_a.png", "start": 135683832, "end": 135684514}, {"filename": "/GameData/textures/lq_dev/wall_green_b.png", "start": 135684514, "end": 135685196}, {"filename": "/GameData/textures/lq_dev/wall_green_c.png", "start": 135685196, "end": 135685878}, {"filename": "/GameData/textures/lq_dev/wall_grey_a.png", "start": 135685878, "end": 135686552}, {"filename": "/GameData/textures/lq_dev/wall_grey_b.png", "start": 135686552, "end": 135687228}, {"filename": "/GameData/textures/lq_dev/wall_grey_c.png", "start": 135687228, "end": 135687905}, {"filename": "/GameData/textures/lq_dev/wall_olive_a.png", "start": 135687905, "end": 135688585}, {"filename": "/GameData/textures/lq_dev/wall_olive_b.png", "start": 135688585, "end": 135689262}, {"filename": "/GameData/textures/lq_dev/wall_olive_c.png", "start": 135689262, "end": 135689941}, {"filename": "/GameData/textures/lq_dev/wall_orange_a.png", "start": 135689941, "end": 135690623}, {"filename": "/GameData/textures/lq_dev/wall_orange_b.png", "start": 135690623, "end": 135691305}, {"filename": "/GameData/textures/lq_dev/wall_orange_c.png", "start": 135691305, "end": 135691986}, {"filename": "/GameData/textures/lq_dev/wall_pink_a.png", "start": 135691986, "end": 135692668}, {"filename": "/GameData/textures/lq_dev/wall_pink_b.png", "start": 135692668, "end": 135693350}, {"filename": "/GameData/textures/lq_dev/wall_pink_c.png", "start": 135693350, "end": 135694032}, {"filename": "/GameData/textures/lq_dev/wall_purple_a.png", "start": 135694032, "end": 135694715}, {"filename": "/GameData/textures/lq_dev/wall_purple_b.png", "start": 135694715, "end": 135695397}, {"filename": "/GameData/textures/lq_dev/wall_purple_c.png", "start": 135695397, "end": 135696079}, {"filename": "/GameData/textures/lq_dev/wall_red_a.png", "start": 135696079, "end": 135696752}, {"filename": "/GameData/textures/lq_dev/wall_red_b.png", "start": 135696752, "end": 135697425}, {"filename": "/GameData/textures/lq_dev/wall_red_c.png", "start": 135697425, "end": 135698099}, {"filename": "/GameData/textures/lq_dev/wall_tan_a.png", "start": 135698099, "end": 135698781}, {"filename": "/GameData/textures/lq_dev/wall_tan_b.png", "start": 135698781, "end": 135699463}, {"filename": "/GameData/textures/lq_dev/wall_tan_c.png", "start": 135699463, "end": 135700145}, {"filename": "/GameData/textures/lq_dev/wall_yellow_a.png", "start": 135700145, "end": 135700827}, {"filename": "/GameData/textures/lq_dev/wall_yellow_b.png", "start": 135700827, "end": 135701509}, {"filename": "/GameData/textures/lq_dev/wall_yellow_c.png", "start": 135701509, "end": 135702190}, {"filename": "/GameData/textures/lq_dev/{char_0_fbr.png", "start": 135702190, "end": 135702367}, {"filename": "/GameData/textures/lq_dev/{char_1_fbr.png", "start": 135702367, "end": 135702527}, {"filename": "/GameData/textures/lq_dev/{char_2_fbr.png", "start": 135702527, "end": 135702699}, {"filename": "/GameData/textures/lq_dev/{char_3_fbr.png", "start": 135702699, "end": 135702878}, {"filename": "/GameData/textures/lq_dev/{char_4_fbr.png", "start": 135702878, "end": 135703041}, {"filename": "/GameData/textures/lq_dev/{char_5_fbr.png", "start": 135703041, "end": 135703214}, {"filename": "/GameData/textures/lq_dev/{char_6_fbr.png", "start": 135703214, "end": 135703386}, {"filename": "/GameData/textures/lq_dev/{char_7_fbr.png", "start": 135703386, "end": 135703540}, {"filename": "/GameData/textures/lq_dev/{char_8_fbr.png", "start": 135703540, "end": 135703708}, {"filename": "/GameData/textures/lq_dev/{char_9_fbr.png", "start": 135703708, "end": 135703875}, {"filename": "/GameData/textures/lq_dev/{char_a_fbr.png", "start": 135703875, "end": 135704039}, {"filename": "/GameData/textures/lq_dev/{char_b_fbr.png", "start": 135704039, "end": 135704209}, {"filename": "/GameData/textures/lq_dev/{char_c_fbr.png", "start": 135704209, "end": 135704359}, {"filename": "/GameData/textures/lq_dev/{char_d_fbr.png", "start": 135704359, "end": 135704525}, {"filename": "/GameData/textures/lq_dev/{char_e_fbr.png", "start": 135704525, "end": 135704693}, {"filename": "/GameData/textures/lq_dev/{char_f_fbr.png", "start": 135704693, "end": 135704853}, {"filename": "/GameData/textures/lq_dev/{char_g_fbr.png", "start": 135704853, "end": 135705025}, {"filename": "/GameData/textures/lq_dev/{char_h_fbr.png", "start": 135705025, "end": 135705184}, {"filename": "/GameData/textures/lq_dev/{char_i_fbr.png", "start": 135705184, "end": 135705343}, {"filename": "/GameData/textures/lq_dev/{char_j_fbr.png", "start": 135705343, "end": 135705509}, {"filename": "/GameData/textures/lq_dev/{char_k_fbr.png", "start": 135705509, "end": 135705703}, {"filename": "/GameData/textures/lq_dev/{char_l_fbr.png", "start": 135705703, "end": 135705844}, {"filename": "/GameData/textures/lq_dev/{char_m_fbr.png", "start": 135705844, "end": 135705993}, {"filename": "/GameData/textures/lq_dev/{char_n_fbr.png", "start": 135705993, "end": 135706176}, {"filename": "/GameData/textures/lq_dev/{char_o_fbr.png", "start": 135706176, "end": 135706326}, {"filename": "/GameData/textures/lq_dev/{char_p_fbr.png", "start": 135706326, "end": 135706484}, {"filename": "/GameData/textures/lq_dev/{char_q_fbr.png", "start": 135706484, "end": 135706650}, {"filename": "/GameData/textures/lq_dev/{char_r_fbr.png", "start": 135706650, "end": 135706822}, {"filename": "/GameData/textures/lq_dev/{char_s_fbr.png", "start": 135706822, "end": 135706998}, {"filename": "/GameData/textures/lq_dev/{char_t_fbr.png", "start": 135706998, "end": 135707146}, {"filename": "/GameData/textures/lq_dev/{char_trans_fbr.png", "start": 135707146, "end": 135707269}, {"filename": "/GameData/textures/lq_dev/{char_u_fbr.png", "start": 135707269, "end": 135707415}, {"filename": "/GameData/textures/lq_dev/{char_v_fbr.png", "start": 135707415, "end": 135707583}, {"filename": "/GameData/textures/lq_dev/{char_w_fbr.png", "start": 135707583, "end": 135707732}, {"filename": "/GameData/textures/lq_dev/{char_x_fbr.png", "start": 135707732, "end": 135707914}, {"filename": "/GameData/textures/lq_dev/{char_y_fbr.png", "start": 135707914, "end": 135708083}, {"filename": "/GameData/textures/lq_dev/{char_z_fbr.png", "start": 135708083, "end": 135708258}, {"filename": "/GameData/textures/lq_dev/{charlow_a_fbr.png", "start": 135708258, "end": 135708413}, {"filename": "/GameData/textures/lq_dev/{charlow_b_fbr.png", "start": 135708413, "end": 135708572}, {"filename": "/GameData/textures/lq_dev/{charlow_c_fbr.png", "start": 135708572, "end": 135708726}, {"filename": "/GameData/textures/lq_dev/{charlow_d_fbr.png", "start": 135708726, "end": 135708887}, {"filename": "/GameData/textures/lq_dev/{charlow_e_fbr.png", "start": 135708887, "end": 135709041}, {"filename": "/GameData/textures/lq_dev/{charlow_f_fbr.png", "start": 135709041, "end": 135709201}, {"filename": "/GameData/textures/lq_dev/{charlow_g_fbr.png", "start": 135709201, "end": 135709359}, {"filename": "/GameData/textures/lq_dev/{charlow_h_fbr.png", "start": 135709359, "end": 135709513}, {"filename": "/GameData/textures/lq_dev/{charlow_i_fbr.png", "start": 135709513, "end": 135709659}, {"filename": "/GameData/textures/lq_dev/{charlow_j_fbr.png", "start": 135709659, "end": 135709819}, {"filename": "/GameData/textures/lq_dev/{charlow_k_fbr.png", "start": 135709819, "end": 135709998}, {"filename": "/GameData/textures/lq_dev/{charlow_l_fbr.png", "start": 135709998, "end": 135710136}, {"filename": "/GameData/textures/lq_dev/{charlow_m_fbr.png", "start": 135710136, "end": 135710289}, {"filename": "/GameData/textures/lq_dev/{charlow_n_fbr.png", "start": 135710289, "end": 135710439}, {"filename": "/GameData/textures/lq_dev/{charlow_o_fbr.png", "start": 135710439, "end": 135710593}, {"filename": "/GameData/textures/lq_dev/{charlow_p_fbr.png", "start": 135710593, "end": 135710748}, {"filename": "/GameData/textures/lq_dev/{charlow_q_fbr.png", "start": 135710748, "end": 135710898}, {"filename": "/GameData/textures/lq_dev/{charlow_r_fbr.png", "start": 135710898, "end": 135711051}, {"filename": "/GameData/textures/lq_dev/{charlow_s_fbr.png", "start": 135711051, "end": 135711203}, {"filename": "/GameData/textures/lq_dev/{charlow_t_fbr.png", "start": 135711203, "end": 135711369}, {"filename": "/GameData/textures/lq_dev/{charlow_u_fbr.png", "start": 135711369, "end": 135711520}, {"filename": "/GameData/textures/lq_dev/{charlow_v_fbr.png", "start": 135711520, "end": 135711694}, {"filename": "/GameData/textures/lq_dev/{charlow_w_fbr.png", "start": 135711694, "end": 135711848}, {"filename": "/GameData/textures/lq_dev/{charlow_x_fbr.png", "start": 135711848, "end": 135712035}, {"filename": "/GameData/textures/lq_dev/{charlow_y_fbr.png", "start": 135712035, "end": 135712209}, {"filename": "/GameData/textures/lq_dev/{charlow_z_fbr.png", "start": 135712209, "end": 135712380}, {"filename": "/GameData/textures/lq_dev/{chars_add_fbr.png", "start": 135712380, "end": 135712543}, {"filename": "/GameData/textures/lq_dev/{chars_and_fbr.png", "start": 135712543, "end": 135712735}, {"filename": "/GameData/textures/lq_dev/{chars_ardown_fbr.png", "start": 135712735, "end": 135712920}, {"filename": "/GameData/textures/lq_dev/{chars_arleft_fbr.png", "start": 135712920, "end": 135713104}, {"filename": "/GameData/textures/lq_dev/{chars_arright_fbr.png", "start": 135713104, "end": 135713289}, {"filename": "/GameData/textures/lq_dev/{chars_arup_fbr.png", "start": 135713289, "end": 135713467}, {"filename": "/GameData/textures/lq_dev/{chars_at_fbr.png", "start": 135713467, "end": 135713624}, {"filename": "/GameData/textures/lq_dev/{chars_brackc1_fbr.png", "start": 135713624, "end": 135713803}, {"filename": "/GameData/textures/lq_dev/{chars_brackc2_fbr.png", "start": 135713803, "end": 135713987}, {"filename": "/GameData/textures/lq_dev/{chars_brackr1_fbr.png", "start": 135713987, "end": 135714154}, {"filename": "/GameData/textures/lq_dev/{chars_brackr2_fbr.png", "start": 135714154, "end": 135714324}, {"filename": "/GameData/textures/lq_dev/{chars_bracks1_fbr.png", "start": 135714324, "end": 135714478}, {"filename": "/GameData/textures/lq_dev/{chars_bracks2_fbr.png", "start": 135714478, "end": 135714631}, {"filename": "/GameData/textures/lq_dev/{chars_caret_fbr.png", "start": 135714631, "end": 135714811}, {"filename": "/GameData/textures/lq_dev/{chars_colon_fbr.png", "start": 135714811, "end": 135714963}, {"filename": "/GameData/textures/lq_dev/{chars_colonsemi_fbr.png", "start": 135714963, "end": 135715130}, {"filename": "/GameData/textures/lq_dev/{chars_comma_fbr.png", "start": 135715130, "end": 135715286}, {"filename": "/GameData/textures/lq_dev/{chars_div_fbr.png", "start": 135715286, "end": 135715455}, {"filename": "/GameData/textures/lq_dev/{chars_dollar_fbr.png", "start": 135715455, "end": 135715631}, {"filename": "/GameData/textures/lq_dev/{chars_equ_fbr.png", "start": 135715631, "end": 135715790}, {"filename": "/GameData/textures/lq_dev/{chars_exclam_fbr.png", "start": 135715790, "end": 135715934}, {"filename": "/GameData/textures/lq_dev/{chars_grave_fbr.png", "start": 135715934, "end": 135716084}, {"filename": "/GameData/textures/lq_dev/{chars_hash_fbr.png", "start": 135716084, "end": 135716269}, {"filename": "/GameData/textures/lq_dev/{chars_heart_fbr.png", "start": 135716269, "end": 135716454}, {"filename": "/GameData/textures/lq_dev/{chars_multi_fbr.png", "start": 135716454, "end": 135716623}, {"filename": "/GameData/textures/lq_dev/{chars_percent_fbr.png", "start": 135716623, "end": 135716829}, {"filename": "/GameData/textures/lq_dev/{chars_perio_fbr.png", "start": 135716829, "end": 135716966}, {"filename": "/GameData/textures/lq_dev/{chars_pipe_fbr.png", "start": 135716966, "end": 135717113}, {"filename": "/GameData/textures/lq_dev/{chars_quest_fbr.png", "start": 135717113, "end": 135717286}, {"filename": "/GameData/textures/lq_dev/{chars_slaback_fbr.png", "start": 135717286, "end": 135717473}, {"filename": "/GameData/textures/lq_dev/{chars_slafoward_fbr.png", "start": 135717473, "end": 135717652}, {"filename": "/GameData/textures/lq_dev/{chars_smile_fbr.png", "start": 135717652, "end": 135717812}, {"filename": "/GameData/textures/lq_dev/{chars_sub_fbr.png", "start": 135717812, "end": 135717959}, {"filename": "/GameData/textures/lq_dev/{chars_sun_fbr.png", "start": 135717959, "end": 135718158}, {"filename": "/GameData/textures/lq_dev/{chars_thngreater_fbr.png", "start": 135718158, "end": 135718351}, {"filename": "/GameData/textures/lq_dev/{chars_thnless_fbr.png", "start": 135718351, "end": 135718538}, {"filename": "/GameData/textures/lq_dev/{chars_tilde_fbr.png", "start": 135718538, "end": 135718707}, {"filename": "/GameData/textures/lq_dev/{chars_unders_fbr.png", "start": 135718707, "end": 135718844}, {"filename": "/GameData/textures/lq_flesh/bone1_1.png", "start": 135718844, "end": 135726251}, {"filename": "/GameData/textures/lq_flesh/bone1_2.png", "start": 135726251, "end": 135735443}, {"filename": "/GameData/textures/lq_flesh/bone2_1.png", "start": 135735443, "end": 135745875}, {"filename": "/GameData/textures/lq_flesh/dopefish_fbr.png", "start": 135745875, "end": 135754105}, {"filename": "/GameData/textures/lq_flesh/flesh_gut1.png", "start": 135754105, "end": 135764420}, {"filename": "/GameData/textures/lq_flesh/flesh_gut2.png", "start": 135764420, "end": 135776220}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_1.png", "start": 135776220, "end": 135789970}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_2.png", "start": 135789970, "end": 135802950}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_3.png", "start": 135802950, "end": 135814983}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_4a.png", "start": 135814983, "end": 135828196}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_4b.png", "start": 135828196, "end": 135841336}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_5a.png", "start": 135841336, "end": 135854980}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_5b.png", "start": 135854980, "end": 135868553}, {"filename": "/GameData/textures/lq_flesh/flesh_rot2_1.png", "start": 135868553, "end": 135881779}, {"filename": "/GameData/textures/lq_flesh/flesh_rot3_1.png", "start": 135881779, "end": 135896725}, {"filename": "/GameData/textures/lq_flesh/flesh_rot3_2.png", "start": 135896725, "end": 135911764}, {"filename": "/GameData/textures/lq_flesh/flesh_rot4_1.png", "start": 135911764, "end": 135926401}, {"filename": "/GameData/textures/lq_flesh/flesh_rot5_1.png", "start": 135926401, "end": 135938669}, {"filename": "/GameData/textures/lq_flesh/flesh_rot6_1.png", "start": 135938669, "end": 135955076}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_1.png", "start": 135955076, "end": 135966594}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_2.png", "start": 135966594, "end": 135979656}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_3.png", "start": 135979656, "end": 135990659}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_4.png", "start": 135990659, "end": 136002071}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_5.png", "start": 136002071, "end": 136012670}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_1.png", "start": 136012670, "end": 136016326}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_2.png", "start": 136016326, "end": 136020423}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_3.png", "start": 136020423, "end": 136023799}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_4.png", "start": 136023799, "end": 136027397}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_5.png", "start": 136027397, "end": 136030787}, {"filename": "/GameData/textures/lq_flesh/fleshtile.png", "start": 136030787, "end": 136041756}, {"filename": "/GameData/textures/lq_flesh/marbred128.png", "start": 136041756, "end": 136053985}, {"filename": "/GameData/textures/lq_flesh/may_eat_eye2_fbr.png", "start": 136053985, "end": 136057858}, {"filename": "/GameData/textures/lq_flesh/may_eat_eye3_fbr.png", "start": 136057858, "end": 136061735}, {"filename": "/GameData/textures/lq_flesh/may_eat_eye_fbr.png", "start": 136061735, "end": 136065610}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_b.png", "start": 136065610, "end": 136078273}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_c.png", "start": 136078273, "end": 136090997}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_hol1.png", "start": 136090997, "end": 136095669}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_lit1_fbr.png", "start": 136095669, "end": 136099739}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_shut1.png", "start": 136099739, "end": 136114705}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_sp.png", "start": 136114705, "end": 136134847}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_sp2.png", "start": 136134847, "end": 136160470}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_te.png", "start": 136160470, "end": 136175576}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_tet.png", "start": 136175576, "end": 136188796}, {"filename": "/GameData/textures/lq_flesh/may_flesh2_b.png", "start": 136188796, "end": 136202319}, {"filename": "/GameData/textures/lq_flesh/may_flesh3_1a.png", "start": 136202319, "end": 136216045}, {"filename": "/GameData/textures/lq_flesh/may_flesh3_1b.png", "start": 136216045, "end": 136252607}, {"filename": "/GameData/textures/lq_flesh/may_flesh3_b.png", "start": 136252607, "end": 136266493}, {"filename": "/GameData/textures/lq_flesh/may_flesh4_det.png", "start": 136266493, "end": 136276804}, {"filename": "/GameData/textures/lq_flesh/may_flesh4a_det.png", "start": 136276804, "end": 136291665}, {"filename": "/GameData/textures/lq_flesh/may_flesh5.png", "start": 136291665, "end": 136302001}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1a.png", "start": 136302001, "end": 136356736}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1b.png", "start": 136356736, "end": 136409454}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1c.png", "start": 136409454, "end": 136453096}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1lit_fbr.png", "start": 136453096, "end": 136470030}, {"filename": "/GameData/textures/lq_flesh/may_flesh_dr1a.png", "start": 136470030, "end": 136526923}, {"filename": "/GameData/textures/lq_flesh/may_skin_eye.png", "start": 136526923, "end": 136538140}, {"filename": "/GameData/textures/lq_flesh/meat-teeth0.png", "start": 136538140, "end": 136581511}, {"filename": "/GameData/textures/lq_flesh/meat-teeth1.png", "start": 136581511, "end": 136626703}, {"filename": "/GameData/textures/lq_flesh/meat_det1.png", "start": 136626703, "end": 136645434}, {"filename": "/GameData/textures/lq_flesh/meat_det2.png", "start": 136645434, "end": 136668005}, {"filename": "/GameData/textures/lq_flesh/meat_pipe1.png", "start": 136668005, "end": 136692429}, {"filename": "/GameData/textures/lq_flesh/plus_0eye.png", "start": 136692429, "end": 136696574}, {"filename": "/GameData/textures/lq_flesh/plus_0flesh2_gl.png", "start": 136696574, "end": 136711754}, {"filename": "/GameData/textures/lq_flesh/plus_0flesh_but1_fbr.png", "start": 136711754, "end": 136716049}, {"filename": "/GameData/textures/lq_flesh/plus_0flesh_but2_fbr.png", "start": 136716049, "end": 136720238}, {"filename": "/GameData/textures/lq_flesh/plus_0flsh_vent.png", "start": 136720238, "end": 136734489}, {"filename": "/GameData/textures/lq_flesh/plus_1eye.png", "start": 136734489, "end": 136738633}, {"filename": "/GameData/textures/lq_flesh/plus_1flesh2_gl.png", "start": 136738633, "end": 136753792}, {"filename": "/GameData/textures/lq_flesh/plus_1flesh_but2_fbr.png", "start": 136753792, "end": 136757987}, {"filename": "/GameData/textures/lq_flesh/plus_2eye.png", "start": 136757987, "end": 136762027}, {"filename": "/GameData/textures/lq_flesh/plus_2flesh2_gl.png", "start": 136762027, "end": 136777203}, {"filename": "/GameData/textures/lq_flesh/plus_2flesh_but2_fbr.png", "start": 136777203, "end": 136781386}, {"filename": "/GameData/textures/lq_flesh/plus_3eye.png", "start": 136781386, "end": 136785434}, {"filename": "/GameData/textures/lq_flesh/plus_3flesh2_gl.png", "start": 136785434, "end": 136800608}, {"filename": "/GameData/textures/lq_flesh/plus_3flesh_but2_fbr.png", "start": 136800608, "end": 136804779}, {"filename": "/GameData/textures/lq_flesh/plus_4eye.png", "start": 136804779, "end": 136808898}, {"filename": "/GameData/textures/lq_flesh/plus_4flesh2_gl.png", "start": 136808898, "end": 136824078}, {"filename": "/GameData/textures/lq_flesh/plus_5eye.png", "start": 136824078, "end": 136828131}, {"filename": "/GameData/textures/lq_flesh/plus_6eye.png", "start": 136828131, "end": 136832179}, {"filename": "/GameData/textures/lq_flesh/plus_7eye.png", "start": 136832179, "end": 136836250}, {"filename": "/GameData/textures/lq_flesh/plus_8eye.png", "start": 136836250, "end": 136840358}, {"filename": "/GameData/textures/lq_flesh/plus_9eye.png", "start": 136840358, "end": 136844506}, {"filename": "/GameData/textures/lq_flesh/plus_aeye.png", "start": 136844506, "end": 136848498}, {"filename": "/GameData/textures/lq_flesh/plus_aflesh_but1.png", "start": 136848498, "end": 136853064}, {"filename": "/GameData/textures/lq_flesh/plus_aflesh_but2.png", "start": 136853064, "end": 136857593}, {"filename": "/GameData/textures/lq_flesh/plus_aflsh_vent.png", "start": 136857593, "end": 136871993}, {"filename": "/GameData/textures/lq_greek/black.png", "start": 136871993, "end": 136872138}, {"filename": "/GameData/textures/lq_greek/grk_arch1.png", "start": 136872138, "end": 136877155}, {"filename": "/GameData/textures/lq_greek/grk_arch1_2.png", "start": 136877155, "end": 136882781}, {"filename": "/GameData/textures/lq_greek/grk_arch1_a.png", "start": 136882781, "end": 136895556}, {"filename": "/GameData/textures/lq_greek/grk_arch1_b.png", "start": 136895556, "end": 136909016}, {"filename": "/GameData/textures/lq_greek/grk_arch1_c.png", "start": 136909016, "end": 136922189}, {"filename": "/GameData/textures/lq_greek/grk_arch1_d.png", "start": 136922189, "end": 136935706}, {"filename": "/GameData/textures/lq_greek/grk_arch2.png", "start": 136935706, "end": 136938988}, {"filename": "/GameData/textures/lq_greek/grk_arch2_2.png", "start": 136938988, "end": 136942310}, {"filename": "/GameData/textures/lq_greek/grk_arch2_a.png", "start": 136942310, "end": 136952713}, {"filename": "/GameData/textures/lq_greek/grk_arch2_b.png", "start": 136952713, "end": 136963230}, {"filename": "/GameData/textures/lq_greek/grk_arch2_c.png", "start": 136963230, "end": 136973359}, {"filename": "/GameData/textures/lq_greek/grk_arch_end.png", "start": 136973359, "end": 136974115}, {"filename": "/GameData/textures/lq_greek/grk_arch_tcap.png", "start": 136974115, "end": 136974829}, {"filename": "/GameData/textures/lq_greek/grk_arch_trim.png", "start": 136974829, "end": 136976301}, {"filename": "/GameData/textures/lq_greek/grk_bl_arch1a.png", "start": 136976301, "end": 136983936}, {"filename": "/GameData/textures/lq_greek/grk_bl_arch1b.png", "start": 136983936, "end": 136992471}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk1a.png", "start": 136992471, "end": 136994943}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk1b.png", "start": 136994943, "end": 136997081}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk2a.png", "start": 136997081, "end": 136999502}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk2b.png", "start": 136999502, "end": 137001962}, {"filename": "/GameData/textures/lq_greek/grk_bl_flt1.png", "start": 137001962, "end": 137004186}, {"filename": "/GameData/textures/lq_greek/grk_bl_fsh1.png", "start": 137004186, "end": 137007112}, {"filename": "/GameData/textures/lq_greek/grk_bl_fsh2.png", "start": 137007112, "end": 137015204}, {"filename": "/GameData/textures/lq_greek/grk_bl_pil1.png", "start": 137015204, "end": 137017991}, {"filename": "/GameData/textures/lq_greek/grk_bl_trim1.png", "start": 137017991, "end": 137020403}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll1.png", "start": 137020403, "end": 137023123}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll2.png", "start": 137023123, "end": 137025849}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll3a.png", "start": 137025849, "end": 137028207}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll3b.png", "start": 137028207, "end": 137030675}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll4b.png", "start": 137030675, "end": 137033241}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5a.png", "start": 137033241, "end": 137035458}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5b.png", "start": 137035458, "end": 137037757}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5c.png", "start": 137037757, "end": 137040080}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5d.png", "start": 137040080, "end": 137042487}, {"filename": "/GameData/textures/lq_greek/grk_brk15.png", "start": 137042487, "end": 137051993}, {"filename": "/GameData/textures/lq_greek/grk_brk15_b.png", "start": 137051993, "end": 137061241}, {"filename": "/GameData/textures/lq_greek/grk_brk15_c.png", "start": 137061241, "end": 137078109}, {"filename": "/GameData/textures/lq_greek/grk_brk15_f.png", "start": 137078109, "end": 137086402}, {"filename": "/GameData/textures/lq_greek/grk_brk15_g.png", "start": 137086402, "end": 137094051}, {"filename": "/GameData/textures/lq_greek/grk_brk16.png", "start": 137094051, "end": 137103327}, {"filename": "/GameData/textures/lq_greek/grk_brk16_a.png", "start": 137103327, "end": 137112606}, {"filename": "/GameData/textures/lq_greek/grk_brk16_f.png", "start": 137112606, "end": 137122311}, {"filename": "/GameData/textures/lq_greek/grk_brk17.png", "start": 137122311, "end": 137157455}, {"filename": "/GameData/textures/lq_greek/grk_brk17_f.png", "start": 137157455, "end": 137170057}, {"filename": "/GameData/textures/lq_greek/grk_but1.png", "start": 137170057, "end": 137171006}, {"filename": "/GameData/textures/lq_greek/grk_det1.png", "start": 137171006, "end": 137174079}, {"filename": "/GameData/textures/lq_greek/grk_door1.png", "start": 137174079, "end": 137189186}, {"filename": "/GameData/textures/lq_greek/grk_door1_f.png", "start": 137189186, "end": 137199221}, {"filename": "/GameData/textures/lq_greek/grk_door2.png", "start": 137199221, "end": 137210081}, {"filename": "/GameData/textures/lq_greek/grk_door3.png", "start": 137210081, "end": 137222738}, {"filename": "/GameData/textures/lq_greek/grk_ebrick10.png", "start": 137222738, "end": 137233365}, {"filename": "/GameData/textures/lq_greek/grk_ebrick10_bl.png", "start": 137233365, "end": 137247899}, {"filename": "/GameData/textures/lq_greek/grk_ebrick22.png", "start": 137247899, "end": 137261765}, {"filename": "/GameData/textures/lq_greek/grk_ebrick23.png", "start": 137261765, "end": 137271131}, {"filename": "/GameData/textures/lq_greek/grk_ebrick24.png", "start": 137271131, "end": 137279324}, {"filename": "/GameData/textures/lq_greek/grk_flr1.png", "start": 137279324, "end": 137279655}, {"filename": "/GameData/textures/lq_greek/grk_flr2.png", "start": 137279655, "end": 137280509}, {"filename": "/GameData/textures/lq_greek/grk_flr3.png", "start": 137280509, "end": 137283105}, {"filename": "/GameData/textures/lq_greek/grk_flr4_1.png", "start": 137283105, "end": 137285235}, {"filename": "/GameData/textures/lq_greek/grk_flr4_2.png", "start": 137285235, "end": 137287497}, {"filename": "/GameData/textures/lq_greek/grk_flr4_3.png", "start": 137287497, "end": 137289860}, {"filename": "/GameData/textures/lq_greek/grk_flr4_4.png", "start": 137289860, "end": 137292155}, {"filename": "/GameData/textures/lq_greek/grk_flr4_5.png", "start": 137292155, "end": 137295787}, {"filename": "/GameData/textures/lq_greek/grk_flr4_6.png", "start": 137295787, "end": 137298793}, {"filename": "/GameData/textures/lq_greek/grk_flr4_8.png", "start": 137298793, "end": 137302128}, {"filename": "/GameData/textures/lq_greek/grk_flr5_1.png", "start": 137302128, "end": 137304715}, {"filename": "/GameData/textures/lq_greek/grk_flr5_2.png", "start": 137304715, "end": 137307209}, {"filename": "/GameData/textures/lq_greek/grk_flr5_3.png", "start": 137307209, "end": 137309951}, {"filename": "/GameData/textures/lq_greek/grk_flr5_4.png", "start": 137309951, "end": 137312254}, {"filename": "/GameData/textures/lq_greek/grk_key_gl2.png", "start": 137312254, "end": 137313278}, {"filename": "/GameData/textures/lq_greek/grk_key_sl2.png", "start": 137313278, "end": 137314279}, {"filename": "/GameData/textures/lq_greek/grk_lion1.png", "start": 137314279, "end": 137324753}, {"filename": "/GameData/textures/lq_greek/grk_lion2.png", "start": 137324753, "end": 137334420}, {"filename": "/GameData/textures/lq_greek/grk_lion3.png", "start": 137334420, "end": 137344963}, {"filename": "/GameData/textures/lq_greek/grk_lion4.png", "start": 137344963, "end": 137356115}, {"filename": "/GameData/textures/lq_greek/grk_met1.png", "start": 137356115, "end": 137366496}, {"filename": "/GameData/textures/lq_greek/grk_met1_trim.png", "start": 137366496, "end": 137377771}, {"filename": "/GameData/textures/lq_greek/grk_met1a_trim.png", "start": 137377771, "end": 137388739}, {"filename": "/GameData/textures/lq_greek/grk_met1b_trim.png", "start": 137388739, "end": 137402852}, {"filename": "/GameData/textures/lq_greek/grk_met2_trim.png", "start": 137402852, "end": 137416121}, {"filename": "/GameData/textures/lq_greek/grk_met_plt.png", "start": 137416121, "end": 137431512}, {"filename": "/GameData/textures/lq_greek/grk_mural1.png", "start": 137431512, "end": 137451370}, {"filename": "/GameData/textures/lq_greek/grk_mural2.png", "start": 137451370, "end": 137474087}, {"filename": "/GameData/textures/lq_greek/grk_mural3.png", "start": 137474087, "end": 137563367}, {"filename": "/GameData/textures/lq_greek/grk_pl1_a.png", "start": 137563367, "end": 137565183}, {"filename": "/GameData/textures/lq_greek/grk_pl1_b.png", "start": 137565183, "end": 137567266}, {"filename": "/GameData/textures/lq_greek/grk_pl2_a.png", "start": 137567266, "end": 137569498}, {"filename": "/GameData/textures/lq_greek/grk_pl2_b.png", "start": 137569498, "end": 137571710}, {"filename": "/GameData/textures/lq_greek/grk_plat1_side.png", "start": 137571710, "end": 137572927}, {"filename": "/GameData/textures/lq_greek/grk_plat1_top.png", "start": 137572927, "end": 137576824}, {"filename": "/GameData/textures/lq_greek/grk_tile2_1.png", "start": 137576824, "end": 137579258}, {"filename": "/GameData/textures/lq_greek/grk_tile2_2.png", "start": 137579258, "end": 137581611}, {"filename": "/GameData/textures/lq_greek/grk_trim1.png", "start": 137581611, "end": 137588368}, {"filename": "/GameData/textures/lq_greek/grk_trim1_3.png", "start": 137588368, "end": 137594982}, {"filename": "/GameData/textures/lq_greek/grk_trim1_3_s.png", "start": 137594982, "end": 137597022}, {"filename": "/GameData/textures/lq_greek/grk_trim1_4_s.png", "start": 137597022, "end": 137599098}, {"filename": "/GameData/textures/lq_greek/grk_trim1_5.png", "start": 137599098, "end": 137606034}, {"filename": "/GameData/textures/lq_greek/grk_trim1_5_s.png", "start": 137606034, "end": 137608254}, {"filename": "/GameData/textures/lq_greek/grk_trim1_6_s.png", "start": 137608254, "end": 137610531}, {"filename": "/GameData/textures/lq_greek/grk_trim1_7_s.png", "start": 137610531, "end": 137613873}, {"filename": "/GameData/textures/lq_greek/grk_trim2.png", "start": 137613873, "end": 137616234}, {"filename": "/GameData/textures/lq_greek/grk_wall1.png", "start": 137616234, "end": 137619266}, {"filename": "/GameData/textures/lq_greek/grk_wall2.png", "start": 137619266, "end": 137621937}, {"filename": "/GameData/textures/lq_greek/grk_wall3.png", "start": 137621937, "end": 137627723}, {"filename": "/GameData/textures/lq_greek/grk_wall3b.png", "start": 137627723, "end": 137654757}, {"filename": "/GameData/textures/lq_greek/grk_win1_a.png", "start": 137654757, "end": 137663896}, {"filename": "/GameData/textures/lq_greek/grk_win1_b.png", "start": 137663896, "end": 137673239}, {"filename": "/GameData/textures/lq_greek/plus_0grk_but1_fbr.png", "start": 137673239, "end": 137674430}, {"filename": "/GameData/textures/lq_greek/plus_0grk_hbut_fbr.png", "start": 137674430, "end": 137675453}, {"filename": "/GameData/textures/lq_greek/plus_1grk_but1_fbr.png", "start": 137675453, "end": 137676628}, {"filename": "/GameData/textures/lq_greek/plus_1grk_hbut_fbr.png", "start": 137676628, "end": 137677709}, {"filename": "/GameData/textures/lq_greek/plus_2grk_but1_fbr.png", "start": 137677709, "end": 137678860}, {"filename": "/GameData/textures/lq_greek/plus_2grk_hbut_fbr.png", "start": 137678860, "end": 137679900}, {"filename": "/GameData/textures/lq_greek/plus_3grk_but1_fbr.png", "start": 137679900, "end": 137681053}, {"filename": "/GameData/textures/lq_greek/plus_3grk_hbut_fbr.png", "start": 137681053, "end": 137682095}, {"filename": "/GameData/textures/lq_greek/plus_agrk_but1.png", "start": 137682095, "end": 137683086}, {"filename": "/GameData/textures/lq_greek/plus_agrk_hbut.png", "start": 137683086, "end": 137684064}, {"filename": "/GameData/textures/lq_health_ammo/ammo_bottom.png", "start": 137684064, "end": 137684994}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_b1_fbr.png", "start": 137684994, "end": 137685602}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_b2_fbr.png", "start": 137685602, "end": 137686214}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_b3_fbr.png", "start": 137686214, "end": 137687021}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_s1_fbr.png", "start": 137687021, "end": 137687614}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_s2_fbr.png", "start": 137687614, "end": 137688604}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_s3_fbr.png", "start": 137688604, "end": 137689237}, {"filename": "/GameData/textures/lq_health_ammo/ammo_fl.png", "start": 137689237, "end": 137689796}, {"filename": "/GameData/textures/lq_health_ammo/ammo_fl2.png", "start": 137689796, "end": 137690351}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_b1_fbr.png", "start": 137690351, "end": 137691105}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_b2_fbr.png", "start": 137691105, "end": 137691790}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_b3.png", "start": 137691790, "end": 137692361}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_s1.png", "start": 137692361, "end": 137692932}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_s2_fbr.png", "start": 137692932, "end": 137693587}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_s3_fbr.png", "start": 137693587, "end": 137694124}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_b1_fbr.png", "start": 137694124, "end": 137694839}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_b2_fbr.png", "start": 137694839, "end": 137695465}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_b3_fbr.png", "start": 137695465, "end": 137695923}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_s1_fbr.png", "start": 137695923, "end": 137696537}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_s2_fbr.png", "start": 137696537, "end": 137696886}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_b1_fbr.png", "start": 137696886, "end": 137697545}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_b2_fbr.png", "start": 137697545, "end": 137698173}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_b3.png", "start": 137698173, "end": 137698950}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_s1_fbr.png", "start": 137698950, "end": 137699456}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_s2_fbr.png", "start": 137699456, "end": 137700034}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_s3_fbr.png", "start": 137700034, "end": 137700585}, {"filename": "/GameData/textures/lq_health_ammo/ammobotsmall.png", "start": 137700585, "end": 137701253}, {"filename": "/GameData/textures/lq_health_ammo/ammotop.png", "start": 137701253, "end": 137701998}, {"filename": "/GameData/textures/lq_health_ammo/ammotopsmall.png", "start": 137701998, "end": 137702549}, {"filename": "/GameData/textures/lq_health_ammo/boom.png", "start": 137702549, "end": 137703567}, {"filename": "/GameData/textures/lq_health_ammo/boomammo_bottom.png", "start": 137703567, "end": 137703933}, {"filename": "/GameData/textures/lq_health_ammo/boomammotop.png", "start": 137703933, "end": 137704275}, {"filename": "/GameData/textures/lq_health_ammo/boomsmall.png", "start": 137704275, "end": 137704950}, {"filename": "/GameData/textures/lq_health_ammo/epboxlarge_fbr.png", "start": 137704950, "end": 137706134}, {"filename": "/GameData/textures/lq_health_ammo/epboxsmall_fbr.png", "start": 137706134, "end": 137707031}, {"filename": "/GameData/textures/lq_health_ammo/explob_s2.png", "start": 137707031, "end": 137707527}, {"filename": "/GameData/textures/lq_health_ammo/hp15_side.png", "start": 137707527, "end": 137708112}, {"filename": "/GameData/textures/lq_health_ammo/hp25_top2.png", "start": 137708112, "end": 137708541}, {"filename": "/GameData/textures/lq_health_ammo/hp_bottom.png", "start": 137708541, "end": 137709448}, {"filename": "/GameData/textures/lq_health_ammo/hp_details.png", "start": 137709448, "end": 137709952}, {"filename": "/GameData/textures/lq_health_ammo/nails.png", "start": 137709952, "end": 137710999}, {"filename": "/GameData/textures/lq_health_ammo/nailssmall.png", "start": 137710999, "end": 137711740}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp100-winq_fbr.png", "start": 137711740, "end": 137714340}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp100_side_fbr.png", "start": 137714340, "end": 137715232}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp100_top_fbr.png", "start": 137715232, "end": 137715963}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp15_top_fbr.png", "start": 137715963, "end": 137716693}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp15winq_fbr.png", "start": 137716693, "end": 137719480}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp25-winq_fbr.png", "start": 137719480, "end": 137722034}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp25_side_fbr.png", "start": 137722034, "end": 137722856}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp25_top_fbr.png", "start": 137722856, "end": 137723590}, {"filename": "/GameData/textures/lq_health_ammo/plus_0explob2_s1_fbr.png", "start": 137723590, "end": 137724693}, {"filename": "/GameData/textures/lq_health_ammo/plus_0explob_s1_fbr.png", "start": 137724693, "end": 137725326}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp100-winq_fbr.png", "start": 137725326, "end": 137727931}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp100_side_fbr.png", "start": 137727931, "end": 137728825}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp100_top_fbr.png", "start": 137728825, "end": 137729559}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp15_top_fbr.png", "start": 137729559, "end": 137730293}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp15winq_fbr.png", "start": 137730293, "end": 137733079}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp25-winq_fbr.png", "start": 137733079, "end": 137735643}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp25_side_fbr.png", "start": 137735643, "end": 137736469}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp25_top_fbr.png", "start": 137736469, "end": 137737208}, {"filename": "/GameData/textures/lq_health_ammo/plus_1explob2_s1_fbr.png", "start": 137737208, "end": 137738315}, {"filename": "/GameData/textures/lq_health_ammo/plus_1explob_s1_fbr.png", "start": 137738315, "end": 137738948}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp100-winq_fbr.png", "start": 137738948, "end": 137741552}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp100_side_fbr.png", "start": 137741552, "end": 137742446}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp25-winq_fbr.png", "start": 137742446, "end": 137745004}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp25_side_fbr.png", "start": 137745004, "end": 137745829}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp25_top_fbr.png", "start": 137745829, "end": 137746567}, {"filename": "/GameData/textures/lq_health_ammo/plus_2explob2_s1_fbr.png", "start": 137746567, "end": 137747663}, {"filename": "/GameData/textures/lq_health_ammo/plus_2explob_s1_fbr.png", "start": 137747663, "end": 137748305}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp100-winq_fbr.png", "start": 137748305, "end": 137750915}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp100_side_fbr.png", "start": 137750915, "end": 137751812}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp25-winq_fbr.png", "start": 137751812, "end": 137754381}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp25_side_fbr.png", "start": 137754381, "end": 137755212}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp25_top_fbr.png", "start": 137755212, "end": 137755954}, {"filename": "/GameData/textures/lq_health_ammo/plus_3explob2_s1_fbr.png", "start": 137755954, "end": 137757050}, {"filename": "/GameData/textures/lq_health_ammo/plus_3explob_s1_fbr.png", "start": 137757050, "end": 137757692}, {"filename": "/GameData/textures/lq_health_ammo/shells.png", "start": 137757692, "end": 137758638}, {"filename": "/GameData/textures/lq_health_ammo/shellssmall.png", "start": 137758638, "end": 137759364}, {"filename": "/GameData/textures/lq_health_ammo/zap.png", "start": 137759364, "end": 137760357}, {"filename": "/GameData/textures/lq_health_ammo/zapsmall.png", "start": 137760357, "end": 137761121}, {"filename": "/GameData/textures/lq_legacy/brick7.png", "start": 137761121, "end": 137764054}, {"filename": "/GameData/textures/lq_legacy/brick8.png", "start": 137764054, "end": 137767141}, {"filename": "/GameData/textures/lq_legacy/button_0.png", "start": 137767141, "end": 137770613}, {"filename": "/GameData/textures/lq_legacy/button_0_grey.png", "start": 137770613, "end": 137773823}, {"filename": "/GameData/textures/lq_legacy/button_0_grn.png", "start": 137773823, "end": 137777158}, {"filename": "/GameData/textures/lq_legacy/button_1.png", "start": 137777158, "end": 137780613}, {"filename": "/GameData/textures/lq_legacy/button_1_grey.png", "start": 137780613, "end": 137783808}, {"filename": "/GameData/textures/lq_legacy/button_1_grn.png", "start": 137783808, "end": 137787109}, {"filename": "/GameData/textures/lq_legacy/floor_temp.png", "start": 137787109, "end": 137789167}, {"filename": "/GameData/textures/lq_legacy/flr.png", "start": 137789167, "end": 137791924}, {"filename": "/GameData/textures/lq_legacy/gardgrass_1.png", "start": 137791924, "end": 137812094}, {"filename": "/GameData/textures/lq_legacy/go-savgx.png", "start": 137812094, "end": 137813843}, {"filename": "/GameData/textures/lq_legacy/grass.png", "start": 137813843, "end": 137817244}, {"filename": "/GameData/textures/lq_legacy/grk_brk15_c_old.png", "start": 137817244, "end": 137827771}, {"filename": "/GameData/textures/lq_legacy/grk_brk17_f_old.png", "start": 137827771, "end": 137836480}, {"filename": "/GameData/textures/lq_legacy/grk_door1_old.png", "start": 137836480, "end": 137847497}, {"filename": "/GameData/textures/lq_legacy/grk_door2_old.png", "start": 137847497, "end": 137856670}, {"filename": "/GameData/textures/lq_legacy/grk_door3_old.png", "start": 137856670, "end": 137865476}, {"filename": "/GameData/textures/lq_legacy/grk_ebrick22_old.png", "start": 137865476, "end": 137874793}, {"filename": "/GameData/textures/lq_legacy/grk_trim1_7_s_old.png", "start": 137874793, "end": 137876998}, {"filename": "/GameData/textures/lq_legacy/ground_1.png", "start": 137876998, "end": 137879179}, {"filename": "/GameData/textures/lq_legacy/leaves.png", "start": 137879179, "end": 137882241}, {"filename": "/GameData/textures/lq_legacy/marble1_4.png", "start": 137882241, "end": 137885144}, {"filename": "/GameData/textures/lq_legacy/marble1_5.png", "start": 137885144, "end": 137888234}, {"filename": "/GameData/textures/lq_legacy/med_cflat1_3.png", "start": 137888234, "end": 137890715}, {"filename": "/GameData/textures/lq_legacy/med_csl_trm3.png", "start": 137890715, "end": 137892040}, {"filename": "/GameData/textures/lq_legacy/med_csl_trm3b.png", "start": 137892040, "end": 137893030}, {"filename": "/GameData/textures/lq_legacy/med_csl_trm3c.png", "start": 137893030, "end": 137894558}, {"filename": "/GameData/textures/lq_legacy/med_flat10.png", "start": 137894558, "end": 137898451}, {"filename": "/GameData/textures/lq_legacy/med_flat11.png", "start": 137898451, "end": 137901656}, {"filename": "/GameData/textures/lq_legacy/med_flat13.png", "start": 137901656, "end": 137912433}, {"filename": "/GameData/textures/lq_legacy/med_flat14.png", "start": 137912433, "end": 137922834}, {"filename": "/GameData/textures/lq_legacy/med_flat9a.png", "start": 137922834, "end": 137966414}, {"filename": "/GameData/textures/lq_legacy/med_flat9b.png", "start": 137966414, "end": 138008313}, {"filename": "/GameData/textures/lq_legacy/med_plaster1.png", "start": 138008313, "end": 138016355}, {"filename": "/GameData/textures/lq_legacy/med_ret_rock1.png", "start": 138016355, "end": 138020174}, {"filename": "/GameData/textures/lq_legacy/med_ret_wood1_old.png", "start": 138020174, "end": 138027255}, {"filename": "/GameData/textures/lq_legacy/med_rmet_key.png", "start": 138027255, "end": 138031216}, {"filename": "/GameData/textures/lq_legacy/med_rock6.png", "start": 138031216, "end": 138076819}, {"filename": "/GameData/textures/lq_legacy/med_rock7.png", "start": 138076819, "end": 138079703}, {"filename": "/GameData/textures/lq_legacy/med_rock8.png", "start": 138079703, "end": 138081588}, {"filename": "/GameData/textures/lq_legacy/med_rough_block.png", "start": 138081588, "end": 138130431}, {"filename": "/GameData/textures/lq_legacy/med_rough_block_f.png", "start": 138130431, "end": 138184976}, {"filename": "/GameData/textures/lq_legacy/med_tile.png", "start": 138184976, "end": 138187112}, {"filename": "/GameData/textures/lq_legacy/med_wall1.png", "start": 138187112, "end": 138196977}, {"filename": "/GameData/textures/lq_legacy/metground_1.png", "start": 138196977, "end": 138199908}, {"filename": "/GameData/textures/lq_legacy/note-savgx.png", "start": 138199908, "end": 138218928}, {"filename": "/GameData/textures/lq_legacy/plus_0button1.png", "start": 138218928, "end": 138222373}, {"filename": "/GameData/textures/lq_legacy/plus_0button2_fbr.png", "start": 138222373, "end": 138223860}, {"filename": "/GameData/textures/lq_legacy/plus_1button2_fbr.png", "start": 138223860, "end": 138225348}, {"filename": "/GameData/textures/lq_legacy/plus_1button3.png", "start": 138225348, "end": 138227228}, {"filename": "/GameData/textures/lq_legacy/plus_abutton1_fbr.png", "start": 138227228, "end": 138230830}, {"filename": "/GameData/textures/lq_legacy/plus_abutton2_fbr.png", "start": 138230830, "end": 138232328}, {"filename": "/GameData/textures/lq_legacy/readme.txt", "start": 138232328, "end": 138232405}, {"filename": "/GameData/textures/lq_legacy/riktoiflat.png", "start": 138232405, "end": 138246524}, {"filename": "/GameData/textures/lq_legacy/riktoiflat_blu.png", "start": 138246524, "end": 138259459}, {"filename": "/GameData/textures/lq_legacy/riktoiflat_grn.png", "start": 138259459, "end": 138271113}, {"filename": "/GameData/textures/lq_legacy/riktoilava.png", "start": 138271113, "end": 138275617}, {"filename": "/GameData/textures/lq_legacy/riktoislime.png", "start": 138275617, "end": 138279322}, {"filename": "/GameData/textures/lq_legacy/riktoitrim.png", "start": 138279322, "end": 138280809}, {"filename": "/GameData/textures/lq_legacy/riktoitrim__purp.png", "start": 138280809, "end": 138282525}, {"filename": "/GameData/textures/lq_legacy/riktoitrim_blu.png", "start": 138282525, "end": 138284255}, {"filename": "/GameData/textures/lq_legacy/riktoiwall.png", "start": 138284255, "end": 138324052}, {"filename": "/GameData/textures/lq_legacy/riktoiwall__purp.png", "start": 138324052, "end": 138360125}, {"filename": "/GameData/textures/lq_legacy/riktoiwall_blu.png", "start": 138360125, "end": 138396186}, {"filename": "/GameData/textures/lq_legacy/riktoiwater.png", "start": 138396186, "end": 138399077}, {"filename": "/GameData/textures/lq_legacy/rune1_fbr.png", "start": 138399077, "end": 138402090}, {"filename": "/GameData/textures/lq_legacy/rune2_fbr.png", "start": 138402090, "end": 138405156}, {"filename": "/GameData/textures/lq_legacy/rune3_fbr.png", "start": 138405156, "end": 138408552}, {"filename": "/GameData/textures/lq_legacy/rune4_fbr.png", "start": 138408552, "end": 138411104}, {"filename": "/GameData/textures/lq_legacy/sign_easy.png", "start": 138411104, "end": 138416056}, {"filename": "/GameData/textures/lq_legacy/sign_empty.png", "start": 138416056, "end": 138420813}, {"filename": "/GameData/textures/lq_legacy/sign_hard.png", "start": 138420813, "end": 138425686}, {"filename": "/GameData/textures/lq_legacy/sign_medium.png", "start": 138425686, "end": 138430696}, {"filename": "/GameData/textures/lq_legacy/sign_metal_1.png", "start": 138430696, "end": 138431908}, {"filename": "/GameData/textures/lq_legacy/sign_metal_2.png", "start": 138431908, "end": 138433503}, {"filename": "/GameData/textures/lq_legacy/sign_nmare.png", "start": 138433503, "end": 138438524}, {"filename": "/GameData/textures/lq_legacy/sky2.png", "start": 138438524, "end": 138454014}, {"filename": "/GameData/textures/lq_legacy/sky4.png", "start": 138454014, "end": 138463526}, {"filename": "/GameData/textures/lq_legacy/sky5_fbr.png", "start": 138463526, "end": 138480679}, {"filename": "/GameData/textures/lq_legacy/sky5a.png", "start": 138480679, "end": 138489690}, {"filename": "/GameData/textures/lq_legacy/sky7.png", "start": 138489690, "end": 138503605}, {"filename": "/GameData/textures/lq_legacy/sky8.png", "start": 138503605, "end": 138520278}, {"filename": "/GameData/textures/lq_legacy/sky8a_fbr.png", "start": 138520278, "end": 138529074}, {"filename": "/GameData/textures/lq_legacy/tile.png", "start": 138529074, "end": 138537890}, {"filename": "/GameData/textures/lq_legacy/tile_blu.png", "start": 138537890, "end": 138546896}, {"filename": "/GameData/textures/lq_legacy/tile_grn.png", "start": 138546896, "end": 138555159}, {"filename": "/GameData/textures/lq_legacy/trim2_blu.png", "start": 138555159, "end": 138556272}, {"filename": "/GameData/textures/lq_legacy/trim2_grn.png", "start": 138556272, "end": 138557336}, {"filename": "/GameData/textures/lq_legacy/ultrasteel1.png", "start": 138557336, "end": 138570226}, {"filename": "/GameData/textures/lq_legacy/ultrasteel2.png", "start": 138570226, "end": 138582020}, {"filename": "/GameData/textures/lq_legacy/vines1_old.png", "start": 138582020, "end": 138586874}, {"filename": "/GameData/textures/lq_legacy/wiz1_4.png", "start": 138586874, "end": 138590695}, {"filename": "/GameData/textures/lq_liquidsky/+0water_f3.png", "start": 138590695, "end": 138593636}, {"filename": "/GameData/textures/lq_liquidsky/+1water_f3.png", "start": 138593636, "end": 138596560}, {"filename": "/GameData/textures/lq_liquidsky/+2water_f3.png", "start": 138596560, "end": 138599478}, {"filename": "/GameData/textures/lq_liquidsky/+3water_f3.png", "start": 138599478, "end": 138602330}, {"filename": "/GameData/textures/lq_liquidsky/plus_0blood_f1.png", "start": 138602330, "end": 138604119}, {"filename": "/GameData/textures/lq_liquidsky/plus_0fslime.png", "start": 138604119, "end": 138618075}, {"filename": "/GameData/textures/lq_liquidsky/plus_0lava_fall3_fbr.png", "start": 138618075, "end": 138627068}, {"filename": "/GameData/textures/lq_liquidsky/plus_0water_f1.png", "start": 138627068, "end": 138628917}, {"filename": "/GameData/textures/lq_liquidsky/plus_0water_f2.png", "start": 138628917, "end": 138630735}, {"filename": "/GameData/textures/lq_liquidsky/plus_0wfall0.png", "start": 138630735, "end": 138640344}, {"filename": "/GameData/textures/lq_liquidsky/plus_1blood_f1.png", "start": 138640344, "end": 138642149}, {"filename": "/GameData/textures/lq_liquidsky/plus_1fslime.png", "start": 138642149, "end": 138655892}, {"filename": "/GameData/textures/lq_liquidsky/plus_1lava_fall3_fbr.png", "start": 138655892, "end": 138665459}, {"filename": "/GameData/textures/lq_liquidsky/plus_1water_f1.png", "start": 138665459, "end": 138667107}, {"filename": "/GameData/textures/lq_liquidsky/plus_1water_f2.png", "start": 138667107, "end": 138668936}, {"filename": "/GameData/textures/lq_liquidsky/plus_1wfall0.png", "start": 138668936, "end": 138678587}, {"filename": "/GameData/textures/lq_liquidsky/plus_2blood_f1.png", "start": 138678587, "end": 138680445}, {"filename": "/GameData/textures/lq_liquidsky/plus_2fslime.png", "start": 138680445, "end": 138694339}, {"filename": "/GameData/textures/lq_liquidsky/plus_2lava_fall3_fbr.png", "start": 138694339, "end": 138703888}, {"filename": "/GameData/textures/lq_liquidsky/plus_2water_f1.png", "start": 138703888, "end": 138705659}, {"filename": "/GameData/textures/lq_liquidsky/plus_2water_f2.png", "start": 138705659, "end": 138707545}, {"filename": "/GameData/textures/lq_liquidsky/plus_2wfall0.png", "start": 138707545, "end": 138717207}, {"filename": "/GameData/textures/lq_liquidsky/plus_3blood_f1.png", "start": 138717207, "end": 138719007}, {"filename": "/GameData/textures/lq_liquidsky/plus_3fslime.png", "start": 138719007, "end": 138732690}, {"filename": "/GameData/textures/lq_liquidsky/plus_3lava_fall3_fbr.png", "start": 138732690, "end": 138742027}, {"filename": "/GameData/textures/lq_liquidsky/plus_3water_f1.png", "start": 138742027, "end": 138743678}, {"filename": "/GameData/textures/lq_liquidsky/plus_3water_f2.png", "start": 138743678, "end": 138745505}, {"filename": "/GameData/textures/lq_liquidsky/plus_3wfall0.png", "start": 138745505, "end": 138755208}, {"filename": "/GameData/textures/lq_liquidsky/plus_4fslime.png", "start": 138755208, "end": 138768872}, {"filename": "/GameData/textures/lq_liquidsky/plus_4lava_fall3_fbr.png", "start": 138768872, "end": 138778245}, {"filename": "/GameData/textures/lq_liquidsky/plus_4wfall0.png", "start": 138778245, "end": 138787824}, {"filename": "/GameData/textures/lq_liquidsky/plus_5fslime.png", "start": 138787824, "end": 138801592}, {"filename": "/GameData/textures/lq_liquidsky/plus_5lava_fall3_fbr.png", "start": 138801592, "end": 138810955}, {"filename": "/GameData/textures/lq_liquidsky/plus_5wfall0.png", "start": 138810955, "end": 138820554}, {"filename": "/GameData/textures/lq_liquidsky/plus_6fslime.png", "start": 138820554, "end": 138834406}, {"filename": "/GameData/textures/lq_liquidsky/plus_6lava_fall3_fbr.png", "start": 138834406, "end": 138843791}, {"filename": "/GameData/textures/lq_liquidsky/plus_6wfall0.png", "start": 138843791, "end": 138853376}, {"filename": "/GameData/textures/lq_liquidsky/plus_7fslime.png", "start": 138853376, "end": 138867227}, {"filename": "/GameData/textures/lq_liquidsky/plus_7lava_fall3_fbr.png", "start": 138867227, "end": 138876742}, {"filename": "/GameData/textures/lq_liquidsky/plus_7wfall0.png", "start": 138876742, "end": 138886411}, {"filename": "/GameData/textures/lq_liquidsky/plus_8wfall0.png", "start": 138886411, "end": 138896073}, {"filename": "/GameData/textures/lq_liquidsky/plus_9wfall0.png", "start": 138896073, "end": 138905695}, {"filename": "/GameData/textures/lq_liquidsky/sky-test.png", "start": 138905695, "end": 138923291}, {"filename": "/GameData/textures/lq_liquidsky/sky-test.xcf", "start": 138923291, "end": 139056556}, {"filename": "/GameData/textures/lq_liquidsky/sky5_blu.png", "start": 139056556, "end": 139069869}, {"filename": "/GameData/textures/lq_liquidsky/sky5_dismal.png", "start": 139069869, "end": 139083052}, {"filename": "/GameData/textures/lq_liquidsky/sky_galx_fbr.png", "start": 139083052, "end": 139105807}, {"filename": "/GameData/textures/lq_liquidsky/sky_galx_spark_fbr.png", "start": 139105807, "end": 139125365}, {"filename": "/GameData/textures/lq_liquidsky/sky_orng.png", "start": 139125365, "end": 139140004}, {"filename": "/GameData/textures/lq_liquidsky/sky_pando.png", "start": 139140004, "end": 139156112}, {"filename": "/GameData/textures/lq_liquidsky/sky_pando2.png", "start": 139156112, "end": 139172647}, {"filename": "/GameData/textures/lq_liquidsky/sky_star.png", "start": 139172647, "end": 139174307}, {"filename": "/GameData/textures/lq_liquidsky/sky_void.png", "start": 139174307, "end": 139175002}, {"filename": "/GameData/textures/lq_liquidsky/sky_wfog_fbr.png", "start": 139175002, "end": 139175908}, {"filename": "/GameData/textures/lq_liquidsky/star_acid.png", "start": 139175908, "end": 139178087}, {"filename": "/GameData/textures/lq_liquidsky/star_blood1.png", "start": 139178087, "end": 139180160}, {"filename": "/GameData/textures/lq_liquidsky/star_lava1_fbr.png", "start": 139180160, "end": 139183365}, {"filename": "/GameData/textures/lq_liquidsky/star_lava2_fbr.png", "start": 139183365, "end": 139187447}, {"filename": "/GameData/textures/lq_liquidsky/star_lava3_fbr.png", "start": 139187447, "end": 139191416}, {"filename": "/GameData/textures/lq_liquidsky/star_lava_void_fbr.png", "start": 139191416, "end": 139195106}, {"filename": "/GameData/textures/lq_liquidsky/star_lavaskip.png", "start": 139195106, "end": 139196219}, {"filename": "/GameData/textures/lq_liquidsky/star_meatgoo2_fbr.png", "start": 139196219, "end": 139199407}, {"filename": "/GameData/textures/lq_liquidsky/star_meatgoo_fbr.png", "start": 139199407, "end": 139202204}, {"filename": "/GameData/textures/lq_liquidsky/star_slime1.png", "start": 139202204, "end": 139208495}, {"filename": "/GameData/textures/lq_liquidsky/star_slime2.png", "start": 139208495, "end": 139211684}, {"filename": "/GameData/textures/lq_liquidsky/star_slime3.png", "start": 139211684, "end": 139213716}, {"filename": "/GameData/textures/lq_liquidsky/star_slime_soul.png", "start": 139213716, "end": 139216506}, {"filename": "/GameData/textures/lq_liquidsky/star_slimeskip.png", "start": 139216506, "end": 139217579}, {"filename": "/GameData/textures/lq_liquidsky/star_soul_drain.png", "start": 139217579, "end": 139220547}, {"filename": "/GameData/textures/lq_liquidsky/star_tele1_fbr.png", "start": 139220547, "end": 139222427}, {"filename": "/GameData/textures/lq_liquidsky/star_tele2_fbr.png", "start": 139222427, "end": 139225228}, {"filename": "/GameData/textures/lq_liquidsky/star_tele3_fbr.png", "start": 139225228, "end": 139228264}, {"filename": "/GameData/textures/lq_liquidsky/star_tele4_fbr.png", "start": 139228264, "end": 139230956}, {"filename": "/GameData/textures/lq_liquidsky/star_water0.png", "start": 139230956, "end": 139233804}, {"filename": "/GameData/textures/lq_liquidsky/star_water1.png", "start": 139233804, "end": 139236913}, {"filename": "/GameData/textures/lq_liquidsky/star_water2.png", "start": 139236913, "end": 139239173}, {"filename": "/GameData/textures/lq_liquidsky/star_water3.png", "start": 139239173, "end": 139241554}, {"filename": "/GameData/textures/lq_liquidsky/star_water4.png", "start": 139241554, "end": 139246341}, {"filename": "/GameData/textures/lq_liquidsky/star_waterskip.png", "start": 139246341, "end": 139248081}, {"filename": "/GameData/textures/lq_liquidsky/star_wstill0.png", "start": 139248081, "end": 139250920}, {"filename": "/GameData/textures/lq_mayan/btn1.png", "start": 139250920, "end": 139253838}, {"filename": "/GameData/textures/lq_mayan/may_arrow.png", "start": 139253838, "end": 139254719}, {"filename": "/GameData/textures/lq_mayan/may_blok1_1.png", "start": 139254719, "end": 139257240}, {"filename": "/GameData/textures/lq_mayan/may_blok1_2.png", "start": 139257240, "end": 139259431}, {"filename": "/GameData/textures/lq_mayan/may_blok1_2_m.png", "start": 139259431, "end": 139262527}, {"filename": "/GameData/textures/lq_mayan/may_blok1_m.png", "start": 139262527, "end": 139271801}, {"filename": "/GameData/textures/lq_mayan/may_blok2_1.png", "start": 139271801, "end": 139274365}, {"filename": "/GameData/textures/lq_mayan/may_blok2_1_m.png", "start": 139274365, "end": 139277586}, {"filename": "/GameData/textures/lq_mayan/may_blok2_2.png", "start": 139277586, "end": 139287849}, {"filename": "/GameData/textures/lq_mayan/may_blok2_2_m.png", "start": 139287849, "end": 139300277}, {"filename": "/GameData/textures/lq_mayan/may_blud1_1.png", "start": 139300277, "end": 139302869}, {"filename": "/GameData/textures/lq_mayan/may_blud1_1m.png", "start": 139302869, "end": 139306089}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_1.png", "start": 139306089, "end": 139308524}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_1m.png", "start": 139308524, "end": 139311594}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_2.png", "start": 139311594, "end": 139314021}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_2m.png", "start": 139314021, "end": 139317049}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_3.png", "start": 139317049, "end": 139319308}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_3m.png", "start": 139319308, "end": 139322717}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_5.png", "start": 139322717, "end": 139325570}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_5m.png", "start": 139325570, "end": 139329219}, {"filename": "/GameData/textures/lq_mayan/may_bnd_skull.png", "start": 139329219, "end": 139331851}, {"filename": "/GameData/textures/lq_mayan/may_brk1_0.png", "start": 139331851, "end": 139334955}, {"filename": "/GameData/textures/lq_mayan/may_brk1_0m.png", "start": 139334955, "end": 139338012}, {"filename": "/GameData/textures/lq_mayan/may_brk1_1.png", "start": 139338012, "end": 139340694}, {"filename": "/GameData/textures/lq_mayan/may_brk1_1m.png", "start": 139340694, "end": 139343893}, {"filename": "/GameData/textures/lq_mayan/may_brk1_2.png", "start": 139343893, "end": 139346844}, {"filename": "/GameData/textures/lq_mayan/may_brk1_2m.png", "start": 139346844, "end": 139350249}, {"filename": "/GameData/textures/lq_mayan/may_brk1_3.png", "start": 139350249, "end": 139353186}, {"filename": "/GameData/textures/lq_mayan/may_brk1_3m.png", "start": 139353186, "end": 139356362}, {"filename": "/GameData/textures/lq_mayan/may_brk2_0.png", "start": 139356362, "end": 139366217}, {"filename": "/GameData/textures/lq_mayan/may_brk2_0_m.png", "start": 139366217, "end": 139378550}, {"filename": "/GameData/textures/lq_mayan/may_brk_old.png", "start": 139378550, "end": 139381149}, {"filename": "/GameData/textures/lq_mayan/may_brk_oldm.png", "start": 139381149, "end": 139384488}, {"filename": "/GameData/textures/lq_mayan/may_deco1_1.png", "start": 139384488, "end": 139395717}, {"filename": "/GameData/textures/lq_mayan/may_deco1_1m.png", "start": 139395717, "end": 139409752}, {"filename": "/GameData/textures/lq_mayan/may_deco1_2.png", "start": 139409752, "end": 139412272}, {"filename": "/GameData/textures/lq_mayan/may_deco1_2my.png", "start": 139412272, "end": 139415660}, {"filename": "/GameData/textures/lq_mayan/may_deco1_3.png", "start": 139415660, "end": 139434233}, {"filename": "/GameData/textures/lq_mayan/may_deco1_3m.png", "start": 139434233, "end": 139454584}, {"filename": "/GameData/textures/lq_mayan/may_door1_1.png", "start": 139454584, "end": 139464051}, {"filename": "/GameData/textures/lq_mayan/may_door1_1m.png", "start": 139464051, "end": 139475142}, {"filename": "/GameData/textures/lq_mayan/may_door2_1.png", "start": 139475142, "end": 139487110}, {"filename": "/GameData/textures/lq_mayan/may_door2_2.png", "start": 139487110, "end": 139498888}, {"filename": "/GameData/textures/lq_mayan/may_drt2_1.png", "start": 139498888, "end": 139501725}, {"filename": "/GameData/textures/lq_mayan/may_flr1_1.png", "start": 139501725, "end": 139504630}, {"filename": "/GameData/textures/lq_mayan/may_flr1_2.png", "start": 139504630, "end": 139507853}, {"filename": "/GameData/textures/lq_mayan/may_flt1_1.png", "start": 139507853, "end": 139510142}, {"filename": "/GameData/textures/lq_mayan/may_flt1_1m.png", "start": 139510142, "end": 139513590}, {"filename": "/GameData/textures/lq_mayan/may_key1_1.png", "start": 139513590, "end": 139515177}, {"filename": "/GameData/textures/lq_mayan/may_key1_2.png", "start": 139515177, "end": 139517174}, {"filename": "/GameData/textures/lq_mayan/may_lite1_1_fbr.png", "start": 139517174, "end": 139518104}, {"filename": "/GameData/textures/lq_mayan/may_lite1_2.png", "start": 139518104, "end": 139518467}, {"filename": "/GameData/textures/lq_mayan/may_lite2_1.png", "start": 139518467, "end": 139521404}, {"filename": "/GameData/textures/lq_mayan/may_lite2_2.png", "start": 139521404, "end": 139521890}, {"filename": "/GameData/textures/lq_mayan/may_lite3_1_fbr.png", "start": 139521890, "end": 139522847}, {"filename": "/GameData/textures/lq_mayan/may_lite_f1.png", "start": 139522847, "end": 139523333}, {"filename": "/GameData/textures/lq_mayan/may_oldmtomb1_1_fbr.png", "start": 139523333, "end": 139537185}, {"filename": "/GameData/textures/lq_mayan/may_oldtomb1_2_fbr.png", "start": 139537185, "end": 139551077}, {"filename": "/GameData/textures/lq_mayan/may_plat_stem_m.png", "start": 139551077, "end": 139551943}, {"filename": "/GameData/textures/lq_mayan/may_plats.png", "start": 139551943, "end": 139554605}, {"filename": "/GameData/textures/lq_mayan/may_platst.png", "start": 139554605, "end": 139557032}, {"filename": "/GameData/textures/lq_mayan/may_platt.png", "start": 139557032, "end": 139559655}, {"filename": "/GameData/textures/lq_mayan/may_plr1_1.png", "start": 139559655, "end": 139562849}, {"filename": "/GameData/textures/lq_mayan/may_tomb1_1_fbr.png", "start": 139562849, "end": 139576701}, {"filename": "/GameData/textures/lq_mayan/may_tomb1_2_fbr.png", "start": 139576701, "end": 139590593}, {"filename": "/GameData/textures/lq_mayan/may_trm1_1.png", "start": 139590593, "end": 139599521}, {"filename": "/GameData/textures/lq_mayan/may_trm1_2.png", "start": 139599521, "end": 139602226}, {"filename": "/GameData/textures/lq_mayan/may_trm1_a.png", "start": 139602226, "end": 139604568}, {"filename": "/GameData/textures/lq_mayan/may_tskull.png", "start": 139604568, "end": 139615574}, {"filename": "/GameData/textures/lq_mayan/may_wall1_1.png", "start": 139615574, "end": 139618556}, {"filename": "/GameData/textures/lq_mayan/may_wall1_2.png", "start": 139618556, "end": 139621522}, {"filename": "/GameData/textures/lq_mayan/may_wall1_3.png", "start": 139621522, "end": 139624716}, {"filename": "/GameData/textures/lq_mayan/may_wall1_3a.png", "start": 139624716, "end": 139627297}, {"filename": "/GameData/textures/lq_mayan/may_wall1_4.png", "start": 139627297, "end": 139630285}, {"filename": "/GameData/textures/lq_mayan/may_wall1_4a.png", "start": 139630285, "end": 139632684}, {"filename": "/GameData/textures/lq_mayan/maya_end_dr1.png", "start": 139632684, "end": 139637070}, {"filename": "/GameData/textures/lq_mayan/maya_end_dr2.png", "start": 139637070, "end": 139641455}, {"filename": "/GameData/textures/lq_mayan/maya_end_trim1.png", "start": 139641455, "end": 139645845}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_btn1.png", "start": 139645845, "end": 139648762}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mpiloilon_fbr.png", "start": 139648762, "end": 139650320}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mpilon_fbr.png", "start": 139650320, "end": 139651895}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mshoohoot_fbr.png", "start": 139651895, "end": 139652840}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mshoot_fbr.png", "start": 139652840, "end": 139653783}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_btn1.png", "start": 139653783, "end": 139656691}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mpiloilon_fbr.png", "start": 139656691, "end": 139658264}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mpilon_fbr.png", "start": 139658264, "end": 139659857}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mshoohoot_fbr.png", "start": 139659857, "end": 139660842}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mshoot_fbr.png", "start": 139660842, "end": 139661826}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_btn1.png", "start": 139661826, "end": 139664717}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_mpilon.png", "start": 139664717, "end": 139666294}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_mshoohoot_fbr.png", "start": 139666294, "end": 139667319}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_mshoot_fbr.png", "start": 139667319, "end": 139668346}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_btn1.png", "start": 139668346, "end": 139671408}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mpiloilon_fbr.png", "start": 139671408, "end": 139672981}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mpilon_fbr.png", "start": 139672981, "end": 139674574}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mshoohoot_fbr.png", "start": 139674574, "end": 139675559}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mshoot_fbr.png", "start": 139675559, "end": 139676543}, {"filename": "/GameData/textures/lq_mayan/plus_4_may_btn1.png", "start": 139676543, "end": 139679606}, {"filename": "/GameData/textures/lq_mayan/plus_5_may_btn1.png", "start": 139679606, "end": 139682347}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_btn1.png", "start": 139682347, "end": 139685070}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_mpiloilon_fbr.png", "start": 139685070, "end": 139686647}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_mpilon_fbr.png", "start": 139686647, "end": 139688231}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_mshoot.png", "start": 139688231, "end": 139689180}, {"filename": "/GameData/textures/lq_medieval/+0med_but1.png", "start": 139689180, "end": 139693435}, {"filename": "/GameData/textures/lq_medieval/+0med_but2.png", "start": 139693435, "end": 139695147}, {"filename": "/GameData/textures/lq_medieval/+0med_but3.png", "start": 139695147, "end": 139696850}, {"filename": "/GameData/textures/lq_medieval/+0med_but_s1.png", "start": 139696850, "end": 139701096}, {"filename": "/GameData/textures/lq_medieval/+0med_sht_but1.png", "start": 139701096, "end": 139702389}, {"filename": "/GameData/textures/lq_medieval/+1med_but3.png", "start": 139702389, "end": 139704091}, {"filename": "/GameData/textures/lq_medieval/+1med_but_s1.png", "start": 139704091, "end": 139708310}, {"filename": "/GameData/textures/lq_medieval/+1med_sht_but1.png", "start": 139708310, "end": 139709623}, {"filename": "/GameData/textures/lq_medieval/+2med_but_s1.png", "start": 139709623, "end": 139713897}, {"filename": "/GameData/textures/lq_medieval/+3med_but_s1.png", "start": 139713897, "end": 139718116}, {"filename": "/GameData/textures/lq_medieval/+amed_but1.png", "start": 139718116, "end": 139722199}, {"filename": "/GameData/textures/lq_medieval/+amed_but2.png", "start": 139722199, "end": 139723917}, {"filename": "/GameData/textures/lq_medieval/+amed_but3.png", "start": 139723917, "end": 139725527}, {"filename": "/GameData/textures/lq_medieval/+amed_but_s1.png", "start": 139725527, "end": 139729740}, {"filename": "/GameData/textures/lq_medieval/+amed_sht_but1.png", "start": 139729740, "end": 139730896}, {"filename": "/GameData/textures/lq_medieval/Art1.png", "start": 139730896, "end": 139863889}, {"filename": "/GameData/textures/lq_medieval/afloor1_4.png", "start": 139863889, "end": 139866890}, {"filename": "/GameData/textures/lq_medieval/afloor1_8.png", "start": 139866890, "end": 139869730}, {"filename": "/GameData/textures/lq_medieval/afloor3_1.png", "start": 139869730, "end": 139872540}, {"filename": "/GameData/textures/lq_medieval/altar1_1.png", "start": 139872540, "end": 139876428}, {"filename": "/GameData/textures/lq_medieval/altar1_3.png", "start": 139876428, "end": 139880909}, {"filename": "/GameData/textures/lq_medieval/altar1_4.png", "start": 139880909, "end": 139884962}, {"filename": "/GameData/textures/lq_medieval/brick0.png", "start": 139884962, "end": 139893184}, {"filename": "/GameData/textures/lq_medieval/brick1.png", "start": 139893184, "end": 139903938}, {"filename": "/GameData/textures/lq_medieval/brick4_s.png", "start": 139903938, "end": 139905871}, {"filename": "/GameData/textures/lq_medieval/brown1.png", "start": 139905871, "end": 139915047}, {"filename": "/GameData/textures/lq_medieval/med_block_1a.png", "start": 139915047, "end": 139961402}, {"filename": "/GameData/textures/lq_medieval/med_block_1b.png", "start": 139961402, "end": 140044766}, {"filename": "/GameData/textures/lq_medieval/med_block_1c.png", "start": 140044766, "end": 140142937}, {"filename": "/GameData/textures/lq_medieval/med_block_1d.png", "start": 140142937, "end": 140251606}, {"filename": "/GameData/textures/lq_medieval/med_block_1e.png", "start": 140251606, "end": 140360099}, {"filename": "/GameData/textures/lq_medieval/med_block_1f.png", "start": 140360099, "end": 140427958}, {"filename": "/GameData/textures/lq_medieval/med_block_1s.png", "start": 140427958, "end": 140545409}, {"filename": "/GameData/textures/lq_medieval/med_block_2a.png", "start": 140545409, "end": 140622793}, {"filename": "/GameData/textures/lq_medieval/med_block_2b.png", "start": 140622793, "end": 140700232}, {"filename": "/GameData/textures/lq_medieval/med_block_2c.png", "start": 140700232, "end": 140789107}, {"filename": "/GameData/textures/lq_medieval/med_block_2d.png", "start": 140789107, "end": 140887301}, {"filename": "/GameData/textures/lq_medieval/med_block_2e.png", "start": 140887301, "end": 140985538}, {"filename": "/GameData/textures/lq_medieval/med_block_2f.png", "start": 140985538, "end": 141050082}, {"filename": "/GameData/textures/lq_medieval/med_block_2s.png", "start": 141050082, "end": 141155834}, {"filename": "/GameData/textures/lq_medieval/med_brk9_ceil1a.png", "start": 141155834, "end": 141167642}, {"filename": "/GameData/textures/lq_medieval/med_brk9_ceil1b.png", "start": 141167642, "end": 141180177}, {"filename": "/GameData/textures/lq_medieval/med_brk9_wal2a.png", "start": 141180177, "end": 141193074}, {"filename": "/GameData/textures/lq_medieval/med_brk9_wal2b.png", "start": 141193074, "end": 141206330}, {"filename": "/GameData/textures/lq_medieval/med_brk9_wal2c.png", "start": 141206330, "end": 141218772}, {"filename": "/GameData/textures/lq_medieval/med_brk9_win1.png", "start": 141218772, "end": 141261770}, {"filename": "/GameData/textures/lq_medieval/med_brk9_win1b.png", "start": 141261770, "end": 141307096}, {"filename": "/GameData/textures/lq_medieval/med_but_side.png", "start": 141307096, "end": 141308094}, {"filename": "/GameData/textures/lq_medieval/med_cmet1.png", "start": 141308094, "end": 141311218}, {"filename": "/GameData/textures/lq_medieval/med_cmet2a.png", "start": 141311218, "end": 141314369}, {"filename": "/GameData/textures/lq_medieval/med_cmet2b.png", "start": 141314369, "end": 141317431}, {"filename": "/GameData/textures/lq_medieval/med_cmet2c.png", "start": 141317431, "end": 141320568}, {"filename": "/GameData/textures/lq_medieval/med_cmet3a.png", "start": 141320568, "end": 141323840}, {"filename": "/GameData/textures/lq_medieval/med_cmet3b.png", "start": 141323840, "end": 141327022}, {"filename": "/GameData/textures/lq_medieval/med_cmet4.png", "start": 141327022, "end": 141329863}, {"filename": "/GameData/textures/lq_medieval/med_cmet5a.png", "start": 141329863, "end": 141333001}, {"filename": "/GameData/textures/lq_medieval/med_cmet5c.png", "start": 141333001, "end": 141336144}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk10.png", "start": 141336144, "end": 141350870}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk10_f.png", "start": 141350870, "end": 141363158}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk10b.png", "start": 141363158, "end": 141377721}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk11.png", "start": 141377721, "end": 141388969}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk12.png", "start": 141388969, "end": 141405918}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk12_f.png", "start": 141405918, "end": 141423251}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk13.png", "start": 141423251, "end": 141479368}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk14.png", "start": 141479368, "end": 141495222}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk14_f.png", "start": 141495222, "end": 141508191}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk14b.png", "start": 141508191, "end": 141521681}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk15.png", "start": 141521681, "end": 141535507}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk15b.png", "start": 141535507, "end": 141549437}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk15f.png", "start": 141549437, "end": 141562635}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk16.png", "start": 141562635, "end": 141578856}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk16b.png", "start": 141578856, "end": 141596448}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk16f.png", "start": 141596448, "end": 141613719}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk17.png", "start": 141613719, "end": 141623826}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk17_f.png", "start": 141623826, "end": 141632851}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk17b.png", "start": 141632851, "end": 141642988}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_f.png", "start": 141642988, "end": 141651621}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_gb.png", "start": 141651621, "end": 141654369}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_gt.png", "start": 141654369, "end": 141657106}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_t.png", "start": 141657106, "end": 141665292}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_tb.png", "start": 141665292, "end": 141675918}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_tc.png", "start": 141675918, "end": 141680447}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18b.png", "start": 141680447, "end": 141688620}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk19_f.png", "start": 141688620, "end": 141700321}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk19_t.png", "start": 141700321, "end": 141711712}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk19b.png", "start": 141711712, "end": 141723032}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk1_1.png", "start": 141723032, "end": 141725517}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk1_2.png", "start": 141725517, "end": 141728688}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk1_3.png", "start": 141728688, "end": 141730931}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk2_1.png", "start": 141730931, "end": 141733480}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk2_2.png", "start": 141733480, "end": 141736853}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk5.png", "start": 141736853, "end": 141740020}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk6_1.png", "start": 141740020, "end": 141742663}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk6_2.png", "start": 141742663, "end": 141747725}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk7_1.png", "start": 141747725, "end": 141750365}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk7_1b.png", "start": 141750365, "end": 141752909}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk7_2.png", "start": 141752909, "end": 141756075}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk8_1c.png", "start": 141756075, "end": 141768680}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk8_1d.png", "start": 141768680, "end": 141781283}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk9_1.png", "start": 141781283, "end": 141794493}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk9_1b.png", "start": 141794493, "end": 141808150}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk9_f.png", "start": 141808150, "end": 141820937}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr1_1.png", "start": 141820937, "end": 141830884}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr2_1.png", "start": 141830884, "end": 141833170}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr2_2.png", "start": 141833170, "end": 141835343}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_1.png", "start": 141835343, "end": 141839083}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_3.png", "start": 141839083, "end": 141842517}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_4.png", "start": 141842517, "end": 141855573}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_5.png", "start": 141855573, "end": 141870091}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr5_1.png", "start": 141870091, "end": 141885021}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr5_2.png", "start": 141885021, "end": 141900549}, {"filename": "/GameData/textures/lq_medieval/med_csl_stp1.png", "start": 141900549, "end": 141903146}, {"filename": "/GameData/textures/lq_medieval/med_csl_stp2.png", "start": 141903146, "end": 141905689}, {"filename": "/GameData/textures/lq_medieval/med_csl_trm1.png", "start": 141905689, "end": 141907014}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1.png", "start": 141907014, "end": 141928487}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t.png", "start": 141928487, "end": 141939874}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t2.png", "start": 141939874, "end": 141953036}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t2b_fbr.png", "start": 141953036, "end": 141966986}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t3.png", "start": 141966986, "end": 141978726}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t4.png", "start": 141978726, "end": 141991502}, {"filename": "/GameData/textures/lq_medieval/med_dbrick2.png", "start": 141991502, "end": 141997036}, {"filename": "/GameData/textures/lq_medieval/med_dbrick3.png", "start": 141997036, "end": 142002410}, {"filename": "/GameData/textures/lq_medieval/med_dbrick4.png", "start": 142002410, "end": 142007718}, {"filename": "/GameData/textures/lq_medieval/med_dbrick5.png", "start": 142007718, "end": 142013540}, {"filename": "/GameData/textures/lq_medieval/med_dbrick6.png", "start": 142013540, "end": 142022701}, {"filename": "/GameData/textures/lq_medieval/med_dbrick6b.png", "start": 142022701, "end": 142031709}, {"filename": "/GameData/textures/lq_medieval/med_dbrick6f.png", "start": 142031709, "end": 142040999}, {"filename": "/GameData/textures/lq_medieval/med_door1.png", "start": 142040999, "end": 142045682}, {"filename": "/GameData/textures/lq_medieval/med_door2.png", "start": 142045682, "end": 142050181}, {"filename": "/GameData/textures/lq_medieval/med_door3.png", "start": 142050181, "end": 142054830}, {"filename": "/GameData/textures/lq_medieval/med_door3b.png", "start": 142054830, "end": 142057371}, {"filename": "/GameData/textures/lq_medieval/med_door4.png", "start": 142057371, "end": 142061935}, {"filename": "/GameData/textures/lq_medieval/med_door4b.png", "start": 142061935, "end": 142064531}, {"filename": "/GameData/textures/lq_medieval/med_dr1a.png", "start": 142064531, "end": 142083315}, {"filename": "/GameData/textures/lq_medieval/med_dr1a_blu.png", "start": 142083315, "end": 142101679}, {"filename": "/GameData/textures/lq_medieval/med_dr1b.png", "start": 142101679, "end": 142125828}, {"filename": "/GameData/textures/lq_medieval/med_dr1b_blu.png", "start": 142125828, "end": 142145447}, {"filename": "/GameData/textures/lq_medieval/med_dr2a.png", "start": 142145447, "end": 142169824}, {"filename": "/GameData/textures/lq_medieval/med_dr2a_blu.png", "start": 142169824, "end": 142189718}, {"filename": "/GameData/textures/lq_medieval/med_dr3a.png", "start": 142189718, "end": 142203201}, {"filename": "/GameData/textures/lq_medieval/med_dr3a_blu.png", "start": 142203201, "end": 142216491}, {"filename": "/GameData/textures/lq_medieval/med_dr3b.png", "start": 142216491, "end": 142230799}, {"filename": "/GameData/textures/lq_medieval/med_dr3b_blu.png", "start": 142230799, "end": 142245005}, {"filename": "/GameData/textures/lq_medieval/med_dr3c.png", "start": 142245005, "end": 142257848}, {"filename": "/GameData/textures/lq_medieval/med_dr3c_blu.png", "start": 142257848, "end": 142270972}, {"filename": "/GameData/textures/lq_medieval/med_dwall1.png", "start": 142270972, "end": 142273448}, {"filename": "/GameData/textures/lq_medieval/med_ebrick1.png", "start": 142273448, "end": 142286030}, {"filename": "/GameData/textures/lq_medieval/med_ebrick10.png", "start": 142286030, "end": 142296915}, {"filename": "/GameData/textures/lq_medieval/med_ebrick10b.png", "start": 142296915, "end": 142307401}, {"filename": "/GameData/textures/lq_medieval/med_ebrick11.png", "start": 142307401, "end": 142322660}, {"filename": "/GameData/textures/lq_medieval/med_ebrick12.png", "start": 142322660, "end": 142340077}, {"filename": "/GameData/textures/lq_medieval/med_ebrick12b.png", "start": 142340077, "end": 142353316}, {"filename": "/GameData/textures/lq_medieval/med_ebrick13.png", "start": 142353316, "end": 142363214}, {"filename": "/GameData/textures/lq_medieval/med_ebrick14.png", "start": 142363214, "end": 142372229}, {"filename": "/GameData/textures/lq_medieval/med_ebrick15.png", "start": 142372229, "end": 142387030}, {"filename": "/GameData/textures/lq_medieval/med_ebrick16.png", "start": 142387030, "end": 142398394}, {"filename": "/GameData/textures/lq_medieval/med_ebrick16b.png", "start": 142398394, "end": 142406712}, {"filename": "/GameData/textures/lq_medieval/med_ebrick17.png", "start": 142406712, "end": 142419846}, {"filename": "/GameData/textures/lq_medieval/med_ebrick17b.png", "start": 142419846, "end": 142432112}, {"filename": "/GameData/textures/lq_medieval/med_ebrick17c.png", "start": 142432112, "end": 142445371}, {"filename": "/GameData/textures/lq_medieval/med_ebrick18.png", "start": 142445371, "end": 142461371}, {"filename": "/GameData/textures/lq_medieval/med_ebrick18b.png", "start": 142461371, "end": 142475759}, {"filename": "/GameData/textures/lq_medieval/med_ebrick2.png", "start": 142475759, "end": 142488251}, {"filename": "/GameData/textures/lq_medieval/med_ebrick20.png", "start": 142488251, "end": 142501651}, {"filename": "/GameData/textures/lq_medieval/med_ebrick21.png", "start": 142501651, "end": 142513793}, {"filename": "/GameData/textures/lq_medieval/med_ebrick22.png", "start": 142513793, "end": 142525954}, {"filename": "/GameData/textures/lq_medieval/med_ebrick3.png", "start": 142525954, "end": 142538414}, {"filename": "/GameData/textures/lq_medieval/med_ebrick4.png", "start": 142538414, "end": 142553964}, {"filename": "/GameData/textures/lq_medieval/med_ebrick5.png", "start": 142553964, "end": 142567780}, {"filename": "/GameData/textures/lq_medieval/med_ebrick6.png", "start": 142567780, "end": 142582491}, {"filename": "/GameData/textures/lq_medieval/med_ebrick7.png", "start": 142582491, "end": 142596136}, {"filename": "/GameData/textures/lq_medieval/med_ebrick8.png", "start": 142596136, "end": 142608875}, {"filename": "/GameData/textures/lq_medieval/med_ebrick9.png", "start": 142608875, "end": 142623430}, {"filename": "/GameData/textures/lq_medieval/med_etrim1.png", "start": 142623430, "end": 142626759}, {"filename": "/GameData/textures/lq_medieval/med_glass1.png", "start": 142626759, "end": 142639138}, {"filename": "/GameData/textures/lq_medieval/med_glass2.png", "start": 142639138, "end": 142650795}, {"filename": "/GameData/textures/lq_medieval/med_glass3.png", "start": 142650795, "end": 142661810}, {"filename": "/GameData/textures/lq_medieval/med_glass4.png", "start": 142661810, "end": 142693629}, {"filename": "/GameData/textures/lq_medieval/med_glass5.png", "start": 142693629, "end": 142703411}, {"filename": "/GameData/textures/lq_medieval/med_met_dec1.png", "start": 142703411, "end": 142707551}, {"filename": "/GameData/textures/lq_medieval/med_met_key1a.png", "start": 142707551, "end": 142709311}, {"filename": "/GameData/textures/lq_medieval/med_met_key1b.png", "start": 142709311, "end": 142710645}, {"filename": "/GameData/textures/lq_medieval/med_met_key2a.png", "start": 142710645, "end": 142712405}, {"filename": "/GameData/textures/lq_medieval/med_met_key2b.png", "start": 142712405, "end": 142714111}, {"filename": "/GameData/textures/lq_medieval/med_met_trim1.png", "start": 142714111, "end": 142715138}, {"filename": "/GameData/textures/lq_medieval/med_met_trim2.png", "start": 142715138, "end": 142716085}, {"filename": "/GameData/textures/lq_medieval/med_met_trim3.png", "start": 142716085, "end": 142716959}, {"filename": "/GameData/textures/lq_medieval/med_metw1a.png", "start": 142716959, "end": 142729267}, {"filename": "/GameData/textures/lq_medieval/med_metw1b.png", "start": 142729267, "end": 142737974}, {"filename": "/GameData/textures/lq_medieval/med_metw2a.png", "start": 142737974, "end": 142747881}, {"filename": "/GameData/textures/lq_medieval/med_metw2b.png", "start": 142747881, "end": 142760259}, {"filename": "/GameData/textures/lq_medieval/med_rmet.png", "start": 142760259, "end": 142773360}, {"filename": "/GameData/textures/lq_medieval/med_rmet_slat.png", "start": 142773360, "end": 142787671}, {"filename": "/GameData/textures/lq_medieval/med_rmet_tile.png", "start": 142787671, "end": 142801603}, {"filename": "/GameData/textures/lq_medieval/med_rmet_trim32.png", "start": 142801603, "end": 142814812}, {"filename": "/GameData/textures/lq_medieval/med_roof1.png", "start": 142814812, "end": 142826727}, {"filename": "/GameData/textures/lq_medieval/med_roof2.png", "start": 142826727, "end": 142841800}, {"filename": "/GameData/textures/lq_medieval/med_roof3.png", "start": 142841800, "end": 142856729}, {"filename": "/GameData/textures/lq_medieval/med_roof4.png", "start": 142856729, "end": 142874543}, {"filename": "/GameData/textures/lq_medieval/med_roof5.png", "start": 142874543, "end": 142885049}, {"filename": "/GameData/textures/lq_medieval/med_tanwall1.png", "start": 142885049, "end": 142926727}, {"filename": "/GameData/textures/lq_medieval/med_tanwall2.png", "start": 142926727, "end": 142986504}, {"filename": "/GameData/textures/lq_medieval/med_tanwall3.png", "start": 142986504, "end": 143036939}, {"filename": "/GameData/textures/lq_medieval/med_tanwall4.png", "start": 143036939, "end": 143086080}, {"filename": "/GameData/textures/lq_medieval/med_tanwall4_f.png", "start": 143086080, "end": 143130033}, {"filename": "/GameData/textures/lq_medieval/med_tanwall6.png", "start": 143130033, "end": 143174578}, {"filename": "/GameData/textures/lq_medieval/med_tanwall7.png", "start": 143174578, "end": 143220171}, {"filename": "/GameData/textures/lq_medieval/med_tanwall8.png", "start": 143220171, "end": 143257769}, {"filename": "/GameData/textures/lq_medieval/med_tanwall9.png", "start": 143257769, "end": 143306849}, {"filename": "/GameData/textures/lq_medieval/med_tanwall9_f.png", "start": 143306849, "end": 143350828}, {"filename": "/GameData/textures/lq_medieval/med_telepad.png", "start": 143350828, "end": 143355455}, {"filename": "/GameData/textures/lq_medieval/med_tile1.png", "start": 143355455, "end": 143428066}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_lit1_fbr.png", "start": 143428066, "end": 143429669}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_lit3_fbr.png", "start": 143429669, "end": 143430974}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_tele.png", "start": 143430974, "end": 143434156}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim1.png", "start": 143434156, "end": 143437322}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim1b.png", "start": 143437322, "end": 143439377}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim2.png", "start": 143439377, "end": 143442917}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim3.png", "start": 143442917, "end": 143446394}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim4.png", "start": 143446394, "end": 143449507}, {"filename": "/GameData/textures/lq_medieval/med_trim1_1.png", "start": 143449507, "end": 143458211}, {"filename": "/GameData/textures/lq_medieval/med_trim2_1.png", "start": 143458211, "end": 143459325}, {"filename": "/GameData/textures/lq_medieval/med_trim3_1.png", "start": 143459325, "end": 143462920}, {"filename": "/GameData/textures/lq_medieval/med_trim3_2.png", "start": 143462920, "end": 143466479}, {"filename": "/GameData/textures/lq_medieval/med_trim3_3.png", "start": 143466479, "end": 143470176}, {"filename": "/GameData/textures/lq_medieval/med_trim3_4.png", "start": 143470176, "end": 143473547}, {"filename": "/GameData/textures/lq_medieval/med_trim4_1.png", "start": 143473547, "end": 143476971}, {"filename": "/GameData/textures/lq_medieval/med_trim4_2.png", "start": 143476971, "end": 143480495}, {"filename": "/GameData/textures/lq_medieval/med_trim4_3.png", "start": 143480495, "end": 143483855}, {"filename": "/GameData/textures/lq_medieval/med_trim4_4.png", "start": 143483855, "end": 143487078}, {"filename": "/GameData/textures/lq_medieval/plus_0_csl_brk14.png", "start": 143487078, "end": 143503391}, {"filename": "/GameData/textures/lq_medieval/plus_1_csl_brk14.png", "start": 143503391, "end": 143519684}, {"filename": "/GameData/textures/lq_medieval/plus_2_csl_brk14.png", "start": 143519684, "end": 143535983}, {"filename": "/GameData/textures/lq_medieval/plus_3_csl_brk14.png", "start": 143535983, "end": 143552225}, {"filename": "/GameData/textures/lq_medieval/plus_4_csl_brk14.png", "start": 143552225, "end": 143568566}, {"filename": "/GameData/textures/lq_medieval/sidewalk.png", "start": 143568566, "end": 143577322}, {"filename": "/GameData/textures/lq_medieval/sq_trim1_2.png", "start": 143577322, "end": 143585604}, {"filename": "/GameData/textures/lq_medieval/sq_trim1_2_s.png", "start": 143585604, "end": 143588179}, {"filename": "/GameData/textures/lq_medieval/tile.png", "start": 143588179, "end": 143591304}, {"filename": "/GameData/textures/lq_medieval/tile1.png", "start": 143591304, "end": 143663917}, {"filename": "/GameData/textures/lq_medieval/wall14_5.png", "start": 143663917, "end": 143667997}, {"filename": "/GameData/textures/lq_medieval/wbrick1_5.png", "start": 143667997, "end": 143671763}, {"filename": "/GameData/textures/lq_medieval/wswamp2_1.png", "start": 143671763, "end": 143674889}, {"filename": "/GameData/textures/lq_medieval/wswamp2_2.png", "start": 143674889, "end": 143678666}, {"filename": "/GameData/textures/lq_metal/gig1_bone.png", "start": 143678666, "end": 143688347}, {"filename": "/GameData/textures/lq_metal/gig1_bone_l.png", "start": 143688347, "end": 143719008}, {"filename": "/GameData/textures/lq_metal/gig1_skull.png", "start": 143719008, "end": 143721672}, {"filename": "/GameData/textures/lq_metal/gig1_skull_l.png", "start": 143721672, "end": 143730457}, {"filename": "/GameData/textures/lq_metal/gig1_spine.png", "start": 143730457, "end": 143779158}, {"filename": "/GameData/textures/lq_metal/gig2_bone.png", "start": 143779158, "end": 143787581}, {"filename": "/GameData/textures/lq_metal/gig2_bone_l.png", "start": 143787581, "end": 143813329}, {"filename": "/GameData/textures/lq_metal/gig2_bone_s.png", "start": 143813329, "end": 143815942}, {"filename": "/GameData/textures/lq_metal/gig2_mouth_s.png", "start": 143815942, "end": 143818357}, {"filename": "/GameData/textures/lq_metal/med_flat8.png", "start": 143818357, "end": 143821215}, {"filename": "/GameData/textures/lq_metal/med_flat9.png", "start": 143821215, "end": 143824203}, {"filename": "/GameData/textures/lq_metal/met_blc_block.png", "start": 143824203, "end": 143827542}, {"filename": "/GameData/textures/lq_metal/met_blc_diam.png", "start": 143827542, "end": 143830432}, {"filename": "/GameData/textures/lq_metal/met_blc_trim28.png", "start": 143830432, "end": 143835475}, {"filename": "/GameData/textures/lq_metal/met_blc_trim32.png", "start": 143835475, "end": 143838621}, {"filename": "/GameData/textures/lq_metal/met_blc_trim32r.png", "start": 143838621, "end": 143841876}, {"filename": "/GameData/textures/lq_metal/met_blc_trim32s.png", "start": 143841876, "end": 143844976}, {"filename": "/GameData/textures/lq_metal/met_blc_trim64.png", "start": 143844976, "end": 143848119}, {"filename": "/GameData/textures/lq_metal/met_blu_block.png", "start": 143848119, "end": 143853139}, {"filename": "/GameData/textures/lq_metal/met_blu_det1.png", "start": 143853139, "end": 143854295}, {"filename": "/GameData/textures/lq_metal/met_blu_diam.png", "start": 143854295, "end": 143857192}, {"filename": "/GameData/textures/lq_metal/met_blu_diam2.png", "start": 143857192, "end": 143859987}, {"filename": "/GameData/textures/lq_metal/met_blu_diamc.png", "start": 143859987, "end": 143863518}, {"filename": "/GameData/textures/lq_metal/met_blu_door1.png", "start": 143863518, "end": 143868201}, {"filename": "/GameData/textures/lq_metal/met_blu_door2.png", "start": 143868201, "end": 143872700}, {"filename": "/GameData/textures/lq_metal/met_blu_door3.png", "start": 143872700, "end": 143877349}, {"filename": "/GameData/textures/lq_metal/met_blu_door4.png", "start": 143877349, "end": 143881913}, {"filename": "/GameData/textures/lq_metal/met_blu_door5.png", "start": 143881913, "end": 143884856}, {"filename": "/GameData/textures/lq_metal/met_blu_door6.png", "start": 143884856, "end": 143887452}, {"filename": "/GameData/textures/lq_metal/met_blu_fac1.png", "start": 143887452, "end": 143889854}, {"filename": "/GameData/textures/lq_metal/met_blu_flat.png", "start": 143889854, "end": 143893288}, {"filename": "/GameData/textures/lq_metal/met_blu_flatst.png", "start": 143893288, "end": 143895517}, {"filename": "/GameData/textures/lq_metal/met_blu_gig1.png", "start": 143895517, "end": 143899636}, {"filename": "/GameData/textures/lq_metal/met_blu_gig2.png", "start": 143899636, "end": 143903249}, {"filename": "/GameData/textures/lq_metal/met_blu_gig2b.png", "start": 143903249, "end": 143909443}, {"filename": "/GameData/textures/lq_metal/met_blu_grate.png", "start": 143909443, "end": 143912498}, {"filename": "/GameData/textures/lq_metal/met_blu_grate2.png", "start": 143912498, "end": 143915329}, {"filename": "/GameData/textures/lq_metal/met_blu_grate3.png", "start": 143915329, "end": 143916989}, {"filename": "/GameData/textures/lq_metal/met_blu_lit1_fbr.png", "start": 143916989, "end": 143919060}, {"filename": "/GameData/textures/lq_metal/met_blu_lit2_fbr.png", "start": 143919060, "end": 143920249}, {"filename": "/GameData/textures/lq_metal/met_blu_lit3.png", "start": 143920249, "end": 143921435}, {"filename": "/GameData/textures/lq_metal/met_blu_lit4.png", "start": 143921435, "end": 143922696}, {"filename": "/GameData/textures/lq_metal/met_blu_lit5.png", "start": 143922696, "end": 143924789}, {"filename": "/GameData/textures/lq_metal/met_blu_pan1.png", "start": 143924789, "end": 143928271}, {"filename": "/GameData/textures/lq_metal/met_blu_pan2.png", "start": 143928271, "end": 143930557}, {"filename": "/GameData/textures/lq_metal/met_blu_pan3.png", "start": 143930557, "end": 143933984}, {"filename": "/GameData/textures/lq_metal/met_blu_rect.png", "start": 143933984, "end": 143937763}, {"filename": "/GameData/textures/lq_metal/met_blu_rivg.png", "start": 143937763, "end": 143941684}, {"filename": "/GameData/textures/lq_metal/met_blu_rivs.png", "start": 143941684, "end": 143945042}, {"filename": "/GameData/textures/lq_metal/met_blu_slat.png", "start": 143945042, "end": 143948828}, {"filename": "/GameData/textures/lq_metal/met_blu_sqr.png", "start": 143948828, "end": 143952599}, {"filename": "/GameData/textures/lq_metal/met_blu_sqrd.png", "start": 143952599, "end": 143956437}, {"filename": "/GameData/textures/lq_metal/met_blu_sqrs.png", "start": 143956437, "end": 143960191}, {"filename": "/GameData/textures/lq_metal/met_blu_stile.png", "start": 143960191, "end": 143963418}, {"filename": "/GameData/textures/lq_metal/met_blu_tile.png", "start": 143963418, "end": 143966867}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16.png", "start": 143966867, "end": 143970571}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16g.png", "start": 143970571, "end": 143974321}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16h.png", "start": 143974321, "end": 143977938}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16s.png", "start": 143977938, "end": 143981106}, {"filename": "/GameData/textures/lq_metal/met_blu_trim28.png", "start": 143981106, "end": 143986403}, {"filename": "/GameData/textures/lq_metal/met_blu_trim32.png", "start": 143986403, "end": 143989853}, {"filename": "/GameData/textures/lq_metal/met_blu_trim32r.png", "start": 143989853, "end": 143993509}, {"filename": "/GameData/textures/lq_metal/met_blu_trim32s.png", "start": 143993509, "end": 143996833}, {"filename": "/GameData/textures/lq_metal/met_blu_trim64.png", "start": 143996833, "end": 144000310}, {"filename": "/GameData/textures/lq_metal/met_blu_vtrim.png", "start": 144000310, "end": 144003880}, {"filename": "/GameData/textures/lq_metal/met_brn2_pat.png", "start": 144003880, "end": 144007729}, {"filename": "/GameData/textures/lq_metal/met_brn_block.png", "start": 144007729, "end": 144011440}, {"filename": "/GameData/textures/lq_metal/met_brn_blockl.png", "start": 144011440, "end": 144014969}, {"filename": "/GameData/textures/lq_metal/met_brn_det1.png", "start": 144014969, "end": 144016132}, {"filename": "/GameData/textures/lq_metal/met_brn_flat.png", "start": 144016132, "end": 144019653}, {"filename": "/GameData/textures/lq_metal/met_brn_grate.png", "start": 144019653, "end": 144023041}, {"filename": "/GameData/textures/lq_metal/met_brn_grate2.png", "start": 144023041, "end": 144026547}, {"filename": "/GameData/textures/lq_metal/met_brn_grate3.png", "start": 144026547, "end": 144028565}, {"filename": "/GameData/textures/lq_metal/met_brn_lit1_fbr.png", "start": 144028565, "end": 144030678}, {"filename": "/GameData/textures/lq_metal/met_brn_lit2_fbr.png", "start": 144030678, "end": 144031866}, {"filename": "/GameData/textures/lq_metal/met_brn_lit3.png", "start": 144031866, "end": 144033037}, {"filename": "/GameData/textures/lq_metal/met_brn_lit4.png", "start": 144033037, "end": 144034247}, {"filename": "/GameData/textures/lq_metal/met_brn_lit5.png", "start": 144034247, "end": 144036300}, {"filename": "/GameData/textures/lq_metal/met_brn_pan1.png", "start": 144036300, "end": 144039763}, {"filename": "/GameData/textures/lq_metal/met_brn_pan2.png", "start": 144039763, "end": 144042039}, {"filename": "/GameData/textures/lq_metal/met_brn_pan3.png", "start": 144042039, "end": 144045516}, {"filename": "/GameData/textures/lq_metal/met_brn_pan4.png", "start": 144045516, "end": 144049379}, {"filename": "/GameData/textures/lq_metal/met_brn_rect.png", "start": 144049379, "end": 144053057}, {"filename": "/GameData/textures/lq_metal/met_brn_rivg.png", "start": 144053057, "end": 144057085}, {"filename": "/GameData/textures/lq_metal/met_brn_rivs.png", "start": 144057085, "end": 144060508}, {"filename": "/GameData/textures/lq_metal/met_brn_signs.png", "start": 144060508, "end": 144065342}, {"filename": "/GameData/textures/lq_metal/met_brn_slat.png", "start": 144065342, "end": 144068985}, {"filename": "/GameData/textures/lq_metal/met_brn_sqr.png", "start": 144068985, "end": 144072676}, {"filename": "/GameData/textures/lq_metal/met_brn_sqrd.png", "start": 144072676, "end": 144076484}, {"filename": "/GameData/textures/lq_metal/met_brn_sqrs.png", "start": 144076484, "end": 144080188}, {"filename": "/GameData/textures/lq_metal/met_brn_stile.png", "start": 144080188, "end": 144084273}, {"filename": "/GameData/textures/lq_metal/met_brn_tile.png", "start": 144084273, "end": 144087654}, {"filename": "/GameData/textures/lq_metal/met_brn_tile2.png", "start": 144087654, "end": 144090987}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16.png", "start": 144090987, "end": 144095093}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16g.png", "start": 144095093, "end": 144099297}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16h.png", "start": 144099297, "end": 144103494}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16s.png", "start": 144103494, "end": 144107117}, {"filename": "/GameData/textures/lq_metal/met_brn_trim32.png", "start": 144107117, "end": 144110604}, {"filename": "/GameData/textures/lq_metal/met_brn_trim32s.png", "start": 144110604, "end": 144113990}, {"filename": "/GameData/textures/lq_metal/met_brn_trim64.png", "start": 144113990, "end": 144117630}, {"filename": "/GameData/textures/lq_metal/met_brn_vtrim.png", "start": 144117630, "end": 144121083}, {"filename": "/GameData/textures/lq_metal/met_cop_flat.png", "start": 144121083, "end": 144124160}, {"filename": "/GameData/textures/lq_metal/met_cop_riv.png", "start": 144124160, "end": 144127761}, {"filename": "/GameData/textures/lq_metal/met_dbrn_flat.png", "start": 144127761, "end": 144131141}, {"filename": "/GameData/textures/lq_metal/met_dbrn_rect.png", "start": 144131141, "end": 144135026}, {"filename": "/GameData/textures/lq_metal/met_dbrn_slat.png", "start": 144135026, "end": 144138841}, {"filename": "/GameData/textures/lq_metal/met_grate.png", "start": 144138841, "end": 144142191}, {"filename": "/GameData/textures/lq_metal/met_grn_block.png", "start": 144142191, "end": 144146116}, {"filename": "/GameData/textures/lq_metal/met_grn_blockl.png", "start": 144146116, "end": 144149845}, {"filename": "/GameData/textures/lq_metal/met_grn_det1.png", "start": 144149845, "end": 144151096}, {"filename": "/GameData/textures/lq_metal/met_grn_fac1.png", "start": 144151096, "end": 144154006}, {"filename": "/GameData/textures/lq_metal/met_grn_flat.png", "start": 144154006, "end": 144157891}, {"filename": "/GameData/textures/lq_metal/met_grn_grate.png", "start": 144157891, "end": 144160953}, {"filename": "/GameData/textures/lq_metal/met_grn_grate2.png", "start": 144160953, "end": 144163936}, {"filename": "/GameData/textures/lq_metal/met_grn_grate3.png", "start": 144163936, "end": 144165680}, {"filename": "/GameData/textures/lq_metal/met_grn_lit1_fbr.png", "start": 144165680, "end": 144167814}, {"filename": "/GameData/textures/lq_metal/met_grn_lit2_fbr.png", "start": 144167814, "end": 144169015}, {"filename": "/GameData/textures/lq_metal/met_grn_lit3.png", "start": 144169015, "end": 144170227}, {"filename": "/GameData/textures/lq_metal/met_grn_lit4.png", "start": 144170227, "end": 144171513}, {"filename": "/GameData/textures/lq_metal/met_grn_lit5.png", "start": 144171513, "end": 144173705}, {"filename": "/GameData/textures/lq_metal/met_grn_pan1.png", "start": 144173705, "end": 144177421}, {"filename": "/GameData/textures/lq_metal/met_grn_pan2.png", "start": 144177421, "end": 144179786}, {"filename": "/GameData/textures/lq_metal/met_grn_pan3.png", "start": 144179786, "end": 144183386}, {"filename": "/GameData/textures/lq_metal/met_grn_rect.png", "start": 144183386, "end": 144187423}, {"filename": "/GameData/textures/lq_metal/met_grn_rivg.png", "start": 144187423, "end": 144191835}, {"filename": "/GameData/textures/lq_metal/met_grn_rivs.png", "start": 144191835, "end": 144195602}, {"filename": "/GameData/textures/lq_metal/met_grn_slat.png", "start": 144195602, "end": 144199643}, {"filename": "/GameData/textures/lq_metal/met_grn_sqr.png", "start": 144199643, "end": 144203641}, {"filename": "/GameData/textures/lq_metal/met_grn_sqrd.png", "start": 144203641, "end": 144207751}, {"filename": "/GameData/textures/lq_metal/met_grn_sqrs.png", "start": 144207751, "end": 144211781}, {"filename": "/GameData/textures/lq_metal/met_grn_stile.png", "start": 144211781, "end": 144215065}, {"filename": "/GameData/textures/lq_metal/met_grn_tile.png", "start": 144215065, "end": 144218326}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16.png", "start": 144218326, "end": 144221938}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16g.png", "start": 144221938, "end": 144226092}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16h.png", "start": 144226092, "end": 144229641}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16s.png", "start": 144229641, "end": 144233152}, {"filename": "/GameData/textures/lq_metal/met_grn_trim28.png", "start": 144233152, "end": 144238087}, {"filename": "/GameData/textures/lq_metal/met_grn_trim28r.png", "start": 144238087, "end": 144243113}, {"filename": "/GameData/textures/lq_metal/met_grn_trim32.png", "start": 144243113, "end": 144246938}, {"filename": "/GameData/textures/lq_metal/met_grn_trim32r.png", "start": 144246938, "end": 144250808}, {"filename": "/GameData/textures/lq_metal/met_grn_trim32s.png", "start": 144250808, "end": 144254454}, {"filename": "/GameData/textures/lq_metal/met_grn_trim64.png", "start": 144254454, "end": 144258111}, {"filename": "/GameData/textures/lq_metal/met_grn_vtrim.png", "start": 144258111, "end": 144261361}, {"filename": "/GameData/textures/lq_metal/met_gry_beam.png", "start": 144261361, "end": 144264375}, {"filename": "/GameData/textures/lq_metal/met_gry_block.png", "start": 144264375, "end": 144267398}, {"filename": "/GameData/textures/lq_metal/met_gry_flat.png", "start": 144267398, "end": 144270132}, {"filename": "/GameData/textures/lq_metal/met_gry_lit1_fbr.png", "start": 144270132, "end": 144271943}, {"filename": "/GameData/textures/lq_metal/met_gry_lit2_fbr.png", "start": 144271943, "end": 144273066}, {"filename": "/GameData/textures/lq_metal/met_gry_lit2b.png", "start": 144273066, "end": 144276516}, {"filename": "/GameData/textures/lq_metal/met_gry_pan1.png", "start": 144276516, "end": 144279612}, {"filename": "/GameData/textures/lq_metal/met_gry_pan2.png", "start": 144279612, "end": 144281765}, {"filename": "/GameData/textures/lq_metal/met_gry_pan3.png", "start": 144281765, "end": 144284868}, {"filename": "/GameData/textures/lq_metal/met_gry_rect.png", "start": 144284868, "end": 144288065}, {"filename": "/GameData/textures/lq_metal/met_gry_signs.png", "start": 144288065, "end": 144291895}, {"filename": "/GameData/textures/lq_metal/met_gry_slat.png", "start": 144291895, "end": 144295311}, {"filename": "/GameData/textures/lq_metal/met_gry_sqr.png", "start": 144295311, "end": 144298145}, {"filename": "/GameData/textures/lq_metal/met_gry_sqrd.png", "start": 144298145, "end": 144301393}, {"filename": "/GameData/textures/lq_metal/met_gry_sqrs.png", "start": 144301393, "end": 144304354}, {"filename": "/GameData/textures/lq_metal/met_gry_trim64.png", "start": 144304354, "end": 144308055}, {"filename": "/GameData/textures/lq_metal/met_lbrn_flat.png", "start": 144308055, "end": 144311481}, {"filename": "/GameData/textures/lq_metal/met_lbrn_rect.png", "start": 144311481, "end": 144315369}, {"filename": "/GameData/textures/lq_metal/met_lbrn_slat.png", "start": 144315369, "end": 144319323}, {"filename": "/GameData/textures/lq_metal/met_lift.png", "start": 144319323, "end": 144322368}, {"filename": "/GameData/textures/lq_metal/met_met7_1.png", "start": 144322368, "end": 144324841}, {"filename": "/GameData/textures/lq_metal/met_mix_beam.png", "start": 144324841, "end": 144328493}, {"filename": "/GameData/textures/lq_metal/met_mix_diam.png", "start": 144328493, "end": 144331682}, {"filename": "/GameData/textures/lq_metal/met_mix_diam2.png", "start": 144331682, "end": 144334881}, {"filename": "/GameData/textures/lq_metal/met_mix_diamc.png", "start": 144334881, "end": 144338981}, {"filename": "/GameData/textures/lq_metal/met_mt1_flat.png", "start": 144338981, "end": 144353271}, {"filename": "/GameData/textures/lq_metal/met_mt1_rect.png", "start": 144353271, "end": 144368981}, {"filename": "/GameData/textures/lq_metal/met_mt1_slat.png", "start": 144368981, "end": 144384737}, {"filename": "/GameData/textures/lq_metal/met_mt1_sqr.png", "start": 144384737, "end": 144391277}, {"filename": "/GameData/textures/lq_metal/met_mt2_flat.png", "start": 144391277, "end": 144407491}, {"filename": "/GameData/textures/lq_metal/met_mt2_rect.png", "start": 144407491, "end": 144424217}, {"filename": "/GameData/textures/lq_metal/met_mt2_slat.png", "start": 144424217, "end": 144440855}, {"filename": "/GameData/textures/lq_metal/met_mt2_sqr.png", "start": 144440855, "end": 144449253}, {"filename": "/GameData/textures/lq_metal/met_mt3_flat.png", "start": 144449253, "end": 144465853}, {"filename": "/GameData/textures/lq_metal/met_mt3_rect.png", "start": 144465853, "end": 144482761}, {"filename": "/GameData/textures/lq_metal/met_mt3_slat.png", "start": 144482761, "end": 144499672}, {"filename": "/GameData/textures/lq_metal/met_mt3_sqr.png", "start": 144499672, "end": 144508207}, {"filename": "/GameData/textures/lq_metal/met_ora_trim64.png", "start": 144508207, "end": 144510986}, {"filename": "/GameData/textures/lq_metal/met_rail_flat.png", "start": 144510986, "end": 144513133}, {"filename": "/GameData/textures/lq_metal/met_rune1_fbr.png", "start": 144513133, "end": 144515937}, {"filename": "/GameData/textures/lq_metal/met_rune_trim32.png", "start": 144515937, "end": 144520278}, {"filename": "/GameData/textures/lq_metal/met_set1.png", "start": 144520278, "end": 144590123}, {"filename": "/GameData/textures/lq_metal/met_shm_flat.png", "start": 144590123, "end": 144593784}, {"filename": "/GameData/textures/lq_metal/met_shm_rect.png", "start": 144593784, "end": 144597656}, {"filename": "/GameData/textures/lq_metal/met_shm_slat.png", "start": 144597656, "end": 144601707}, {"filename": "/GameData/textures/lq_metal/met_shm_sqr.png", "start": 144601707, "end": 144605169}, {"filename": "/GameData/textures/lq_metal/met_teal_block.png", "start": 144605169, "end": 144608547}, {"filename": "/GameData/textures/lq_metal/met_teal_flat.png", "start": 144608547, "end": 144611893}, {"filename": "/GameData/textures/lq_metal/met_teal_trim32.png", "start": 144611893, "end": 144615169}, {"filename": "/GameData/textures/lq_metal/met_teal_trim32r.png", "start": 144615169, "end": 144618144}, {"filename": "/GameData/textures/lq_metal/met_teal_trim64.png", "start": 144618144, "end": 144621628}, {"filename": "/GameData/textures/lq_metal/met_wall3_1.png", "start": 144621628, "end": 144632894}, {"filename": "/GameData/textures/lq_metal/met_wall3_1_s.png", "start": 144632894, "end": 144636178}, {"filename": "/GameData/textures/lq_metal/metal4_4.png", "start": 144636178, "end": 144640519}, {"filename": "/GameData/textures/lq_metal/plus_0_sqbut1.png", "start": 144640519, "end": 144641601}, {"filename": "/GameData/textures/lq_metal/plus_0_sqbut2_fbr.png", "start": 144641601, "end": 144644837}, {"filename": "/GameData/textures/lq_metal/plus_0_sqshoot1_fbr.png", "start": 144644837, "end": 144645944}, {"filename": "/GameData/textures/lq_metal/plus_0gig2a_fbr.png", "start": 144645944, "end": 144646417}, {"filename": "/GameData/textures/lq_metal/plus_0gig_shot_fbr.png", "start": 144646417, "end": 144646893}, {"filename": "/GameData/textures/lq_metal/plus_0gig_sshot_fbr.png", "start": 144646893, "end": 144647337}, {"filename": "/GameData/textures/lq_metal/plus_0gig_ye_fbr.png", "start": 144647337, "end": 144647777}, {"filename": "/GameData/textures/lq_metal/plus_0met_blu_keyg_fbr.png", "start": 144647777, "end": 144648677}, {"filename": "/GameData/textures/lq_metal/plus_0met_blu_keys_fbr.png", "start": 144648677, "end": 144649560}, {"filename": "/GameData/textures/lq_metal/plus_1_sqbut1.png", "start": 144649560, "end": 144650726}, {"filename": "/GameData/textures/lq_metal/plus_1_sqbut2_fbr.png", "start": 144650726, "end": 144653914}, {"filename": "/GameData/textures/lq_metal/plus_1_sqshoot1.png", "start": 144653914, "end": 144655028}, {"filename": "/GameData/textures/lq_metal/plus_1met_blu_keyg_fbr.png", "start": 144655028, "end": 144655929}, {"filename": "/GameData/textures/lq_metal/plus_1met_blu_keys_fbr.png", "start": 144655929, "end": 144656806}, {"filename": "/GameData/textures/lq_metal/plus_2met_blu_keyg_fbr.png", "start": 144656806, "end": 144657715}, {"filename": "/GameData/textures/lq_metal/plus_2met_blu_keys_fbr.png", "start": 144657715, "end": 144658584}, {"filename": "/GameData/textures/lq_metal/plus_3met_blu_keyg_fbr.png", "start": 144658584, "end": 144659492}, {"filename": "/GameData/textures/lq_metal/plus_3met_blu_keys_fbr.png", "start": 144659492, "end": 144660348}, {"filename": "/GameData/textures/lq_metal/plus_4met_blu_keyg_fbr.png", "start": 144660348, "end": 144661256}, {"filename": "/GameData/textures/lq_metal/plus_4met_blu_keys_fbr.png", "start": 144661256, "end": 144662112}, {"filename": "/GameData/textures/lq_metal/plus_5met_blu_keyg_fbr.png", "start": 144662112, "end": 144663021}, {"filename": "/GameData/textures/lq_metal/plus_5met_blu_keys_fbr.png", "start": 144663021, "end": 144663890}, {"filename": "/GameData/textures/lq_metal/plus_6met_blu_keyg_fbr.png", "start": 144663890, "end": 144664791}, {"filename": "/GameData/textures/lq_metal/plus_6met_blu_keys_fbr.png", "start": 144664791, "end": 144665668}, {"filename": "/GameData/textures/lq_metal/plus_a_sqbut1.png", "start": 144665668, "end": 144666750}, {"filename": "/GameData/textures/lq_metal/plus_a_sqbut2_fbr.png", "start": 144666750, "end": 144669986}, {"filename": "/GameData/textures/lq_metal/plus_a_sqshoot1_fbr.png", "start": 144669986, "end": 144671093}, {"filename": "/GameData/textures/lq_metal/plus_agig2a.png", "start": 144671093, "end": 144671463}, {"filename": "/GameData/textures/lq_metal/plus_agig_shot_fbr.png", "start": 144671463, "end": 144671838}, {"filename": "/GameData/textures/lq_metal/plus_agig_sshot_fbr.png", "start": 144671838, "end": 144672201}, {"filename": "/GameData/textures/lq_metal/plus_agig_ye.png", "start": 144672201, "end": 144672613}, {"filename": "/GameData/textures/lq_metal/plus_amet_blu_keyg.png", "start": 144672613, "end": 144673483}, {"filename": "/GameData/textures/lq_metal/plus_amet_blu_keys.png", "start": 144673483, "end": 144674353}, {"filename": "/GameData/textures/lq_metal/ret_metal1_tile.png", "start": 144674353, "end": 144688285}, {"filename": "/GameData/textures/lq_metal/sq_lit1_fbr.png", "start": 144688285, "end": 144688658}, {"filename": "/GameData/textures/lq_metal/sq_lit2_fbr.png", "start": 144688658, "end": 144688919}, {"filename": "/GameData/textures/lq_palette/flat_01_a.png", "start": 144688919, "end": 144689467}, {"filename": "/GameData/textures/lq_palette/flat_01_b.png", "start": 144689467, "end": 144689993}, {"filename": "/GameData/textures/lq_palette/flat_01_c.png", "start": 144689993, "end": 144690519}, {"filename": "/GameData/textures/lq_palette/flat_01_d.png", "start": 144690519, "end": 144691045}, {"filename": "/GameData/textures/lq_palette/flat_01_e.png", "start": 144691045, "end": 144691571}, {"filename": "/GameData/textures/lq_palette/flat_01_f.png", "start": 144691571, "end": 144692097}, {"filename": "/GameData/textures/lq_palette/flat_01_g.png", "start": 144692097, "end": 144692623}, {"filename": "/GameData/textures/lq_palette/flat_01_h.png", "start": 144692623, "end": 144693149}, {"filename": "/GameData/textures/lq_palette/flat_01_i.png", "start": 144693149, "end": 144693676}, {"filename": "/GameData/textures/lq_palette/flat_01_j.png", "start": 144693676, "end": 144694203}, {"filename": "/GameData/textures/lq_palette/flat_01_k.png", "start": 144694203, "end": 144694730}, {"filename": "/GameData/textures/lq_palette/flat_01_l.png", "start": 144694730, "end": 144695257}, {"filename": "/GameData/textures/lq_palette/flat_01_m.png", "start": 144695257, "end": 144695784}, {"filename": "/GameData/textures/lq_palette/flat_01_n.png", "start": 144695784, "end": 144696309}, {"filename": "/GameData/textures/lq_palette/flat_01_o.png", "start": 144696309, "end": 144696834}, {"filename": "/GameData/textures/lq_palette/flat_01_p.png", "start": 144696834, "end": 144697359}, {"filename": "/GameData/textures/lq_palette/flat_02_a.png", "start": 144697359, "end": 144697885}, {"filename": "/GameData/textures/lq_palette/flat_02_b.png", "start": 144697885, "end": 144698411}, {"filename": "/GameData/textures/lq_palette/flat_02_c.png", "start": 144698411, "end": 144698937}, {"filename": "/GameData/textures/lq_palette/flat_02_d.png", "start": 144698937, "end": 144699463}, {"filename": "/GameData/textures/lq_palette/flat_02_e.png", "start": 144699463, "end": 144699989}, {"filename": "/GameData/textures/lq_palette/flat_02_f.png", "start": 144699989, "end": 144700515}, {"filename": "/GameData/textures/lq_palette/flat_02_g.png", "start": 144700515, "end": 144701041}, {"filename": "/GameData/textures/lq_palette/flat_02_h.png", "start": 144701041, "end": 144701567}, {"filename": "/GameData/textures/lq_palette/flat_02_i.png", "start": 144701567, "end": 144702093}, {"filename": "/GameData/textures/lq_palette/flat_02_j.png", "start": 144702093, "end": 144702619}, {"filename": "/GameData/textures/lq_palette/flat_02_k.png", "start": 144702619, "end": 144703145}, {"filename": "/GameData/textures/lq_palette/flat_02_l.png", "start": 144703145, "end": 144703671}, {"filename": "/GameData/textures/lq_palette/flat_02_m.png", "start": 144703671, "end": 144704197}, {"filename": "/GameData/textures/lq_palette/flat_02_n.png", "start": 144704197, "end": 144704723}, {"filename": "/GameData/textures/lq_palette/flat_02_o.png", "start": 144704723, "end": 144705249}, {"filename": "/GameData/textures/lq_palette/flat_02_p.png", "start": 144705249, "end": 144705775}, {"filename": "/GameData/textures/lq_palette/flat_03_a.png", "start": 144705775, "end": 144706301}, {"filename": "/GameData/textures/lq_palette/flat_03_b.png", "start": 144706301, "end": 144706827}, {"filename": "/GameData/textures/lq_palette/flat_03_c.png", "start": 144706827, "end": 144707353}, {"filename": "/GameData/textures/lq_palette/flat_03_d.png", "start": 144707353, "end": 144707879}, {"filename": "/GameData/textures/lq_palette/flat_03_e.png", "start": 144707879, "end": 144708405}, {"filename": "/GameData/textures/lq_palette/flat_03_f.png", "start": 144708405, "end": 144708931}, {"filename": "/GameData/textures/lq_palette/flat_03_g.png", "start": 144708931, "end": 144709457}, {"filename": "/GameData/textures/lq_palette/flat_03_h.png", "start": 144709457, "end": 144709983}, {"filename": "/GameData/textures/lq_palette/flat_03_i.png", "start": 144709983, "end": 144710509}, {"filename": "/GameData/textures/lq_palette/flat_03_j.png", "start": 144710509, "end": 144711035}, {"filename": "/GameData/textures/lq_palette/flat_03_k.png", "start": 144711035, "end": 144711561}, {"filename": "/GameData/textures/lq_palette/flat_03_l.png", "start": 144711561, "end": 144712087}, {"filename": "/GameData/textures/lq_palette/flat_03_m.png", "start": 144712087, "end": 144712614}, {"filename": "/GameData/textures/lq_palette/flat_03_n.png", "start": 144712614, "end": 144713141}, {"filename": "/GameData/textures/lq_palette/flat_03_o.png", "start": 144713141, "end": 144713668}, {"filename": "/GameData/textures/lq_palette/flat_03_p.png", "start": 144713668, "end": 144714195}, {"filename": "/GameData/textures/lq_palette/flat_04_a.png", "start": 144714195, "end": 144714743}, {"filename": "/GameData/textures/lq_palette/flat_04_b.png", "start": 144714743, "end": 144715269}, {"filename": "/GameData/textures/lq_palette/flat_04_c.png", "start": 144715269, "end": 144715795}, {"filename": "/GameData/textures/lq_palette/flat_04_d.png", "start": 144715795, "end": 144716321}, {"filename": "/GameData/textures/lq_palette/flat_04_e.png", "start": 144716321, "end": 144716847}, {"filename": "/GameData/textures/lq_palette/flat_04_f.png", "start": 144716847, "end": 144717373}, {"filename": "/GameData/textures/lq_palette/flat_04_g.png", "start": 144717373, "end": 144717899}, {"filename": "/GameData/textures/lq_palette/flat_04_h.png", "start": 144717899, "end": 144718425}, {"filename": "/GameData/textures/lq_palette/flat_04_i.png", "start": 144718425, "end": 144718951}, {"filename": "/GameData/textures/lq_palette/flat_04_j.png", "start": 144718951, "end": 144719477}, {"filename": "/GameData/textures/lq_palette/flat_04_k.png", "start": 144719477, "end": 144720003}, {"filename": "/GameData/textures/lq_palette/flat_04_l.png", "start": 144720003, "end": 144720529}, {"filename": "/GameData/textures/lq_palette/flat_04_m.png", "start": 144720529, "end": 144721055}, {"filename": "/GameData/textures/lq_palette/flat_04_n.png", "start": 144721055, "end": 144721581}, {"filename": "/GameData/textures/lq_palette/flat_04_o.png", "start": 144721581, "end": 144722107}, {"filename": "/GameData/textures/lq_palette/flat_04_p.png", "start": 144722107, "end": 144722633}, {"filename": "/GameData/textures/lq_palette/flat_05_a.png", "start": 144722633, "end": 144723159}, {"filename": "/GameData/textures/lq_palette/flat_05_b.png", "start": 144723159, "end": 144723685}, {"filename": "/GameData/textures/lq_palette/flat_05_c.png", "start": 144723685, "end": 144724211}, {"filename": "/GameData/textures/lq_palette/flat_05_d.png", "start": 144724211, "end": 144724737}, {"filename": "/GameData/textures/lq_palette/flat_05_e.png", "start": 144724737, "end": 144725263}, {"filename": "/GameData/textures/lq_palette/flat_05_f.png", "start": 144725263, "end": 144725789}, {"filename": "/GameData/textures/lq_palette/flat_05_g.png", "start": 144725789, "end": 144726315}, {"filename": "/GameData/textures/lq_palette/flat_05_h.png", "start": 144726315, "end": 144726841}, {"filename": "/GameData/textures/lq_palette/flat_05_i.png", "start": 144726841, "end": 144727367}, {"filename": "/GameData/textures/lq_palette/flat_05_j.png", "start": 144727367, "end": 144727893}, {"filename": "/GameData/textures/lq_palette/flat_05_k.png", "start": 144727893, "end": 144728419}, {"filename": "/GameData/textures/lq_palette/flat_05_l.png", "start": 144728419, "end": 144728945}, {"filename": "/GameData/textures/lq_palette/flat_05_m.png", "start": 144728945, "end": 144729471}, {"filename": "/GameData/textures/lq_palette/flat_05_n.png", "start": 144729471, "end": 144729997}, {"filename": "/GameData/textures/lq_palette/flat_05_o.png", "start": 144729997, "end": 144730523}, {"filename": "/GameData/textures/lq_palette/flat_05_p.png", "start": 144730523, "end": 144731049}, {"filename": "/GameData/textures/lq_palette/flat_06_a.png", "start": 144731049, "end": 144731575}, {"filename": "/GameData/textures/lq_palette/flat_06_b.png", "start": 144731575, "end": 144732101}, {"filename": "/GameData/textures/lq_palette/flat_06_c.png", "start": 144732101, "end": 144732627}, {"filename": "/GameData/textures/lq_palette/flat_06_d.png", "start": 144732627, "end": 144733153}, {"filename": "/GameData/textures/lq_palette/flat_06_e.png", "start": 144733153, "end": 144733679}, {"filename": "/GameData/textures/lq_palette/flat_06_f.png", "start": 144733679, "end": 144734205}, {"filename": "/GameData/textures/lq_palette/flat_06_g.png", "start": 144734205, "end": 144734731}, {"filename": "/GameData/textures/lq_palette/flat_06_h.png", "start": 144734731, "end": 144735257}, {"filename": "/GameData/textures/lq_palette/flat_06_i.png", "start": 144735257, "end": 144735783}, {"filename": "/GameData/textures/lq_palette/flat_06_j.png", "start": 144735783, "end": 144736309}, {"filename": "/GameData/textures/lq_palette/flat_06_k.png", "start": 144736309, "end": 144736835}, {"filename": "/GameData/textures/lq_palette/flat_06_l.png", "start": 144736835, "end": 144737361}, {"filename": "/GameData/textures/lq_palette/flat_06_m.png", "start": 144737361, "end": 144737887}, {"filename": "/GameData/textures/lq_palette/flat_06_n.png", "start": 144737887, "end": 144738413}, {"filename": "/GameData/textures/lq_palette/flat_06_o.png", "start": 144738413, "end": 144738939}, {"filename": "/GameData/textures/lq_palette/flat_06_p.png", "start": 144738939, "end": 144739465}, {"filename": "/GameData/textures/lq_palette/flat_07_a.png", "start": 144739465, "end": 144739991}, {"filename": "/GameData/textures/lq_palette/flat_07_b.png", "start": 144739991, "end": 144740517}, {"filename": "/GameData/textures/lq_palette/flat_07_c.png", "start": 144740517, "end": 144741043}, {"filename": "/GameData/textures/lq_palette/flat_07_d.png", "start": 144741043, "end": 144741569}, {"filename": "/GameData/textures/lq_palette/flat_07_e.png", "start": 144741569, "end": 144742095}, {"filename": "/GameData/textures/lq_palette/flat_07_f.png", "start": 144742095, "end": 144742621}, {"filename": "/GameData/textures/lq_palette/flat_07_g.png", "start": 144742621, "end": 144743147}, {"filename": "/GameData/textures/lq_palette/flat_07_h.png", "start": 144743147, "end": 144743673}, {"filename": "/GameData/textures/lq_palette/flat_07_i.png", "start": 144743673, "end": 144744199}, {"filename": "/GameData/textures/lq_palette/flat_07_j.png", "start": 144744199, "end": 144744725}, {"filename": "/GameData/textures/lq_palette/flat_07_k.png", "start": 144744725, "end": 144745251}, {"filename": "/GameData/textures/lq_palette/flat_07_l.png", "start": 144745251, "end": 144745777}, {"filename": "/GameData/textures/lq_palette/flat_07_m.png", "start": 144745777, "end": 144746303}, {"filename": "/GameData/textures/lq_palette/flat_07_n.png", "start": 144746303, "end": 144746829}, {"filename": "/GameData/textures/lq_palette/flat_07_o.png", "start": 144746829, "end": 144747355}, {"filename": "/GameData/textures/lq_palette/flat_07_p.png", "start": 144747355, "end": 144747881}, {"filename": "/GameData/textures/lq_palette/flat_08_a.png", "start": 144747881, "end": 144748407}, {"filename": "/GameData/textures/lq_palette/flat_08_b.png", "start": 144748407, "end": 144748933}, {"filename": "/GameData/textures/lq_palette/flat_08_c.png", "start": 144748933, "end": 144749459}, {"filename": "/GameData/textures/lq_palette/flat_08_d.png", "start": 144749459, "end": 144749985}, {"filename": "/GameData/textures/lq_palette/flat_08_e.png", "start": 144749985, "end": 144750511}, {"filename": "/GameData/textures/lq_palette/flat_08_f.png", "start": 144750511, "end": 144751037}, {"filename": "/GameData/textures/lq_palette/flat_08_g.png", "start": 144751037, "end": 144751563}, {"filename": "/GameData/textures/lq_palette/flat_08_h.png", "start": 144751563, "end": 144752089}, {"filename": "/GameData/textures/lq_palette/flat_08_i.png", "start": 144752089, "end": 144752615}, {"filename": "/GameData/textures/lq_palette/flat_08_j.png", "start": 144752615, "end": 144753141}, {"filename": "/GameData/textures/lq_palette/flat_08_k.png", "start": 144753141, "end": 144753667}, {"filename": "/GameData/textures/lq_palette/flat_08_l.png", "start": 144753667, "end": 144754193}, {"filename": "/GameData/textures/lq_palette/flat_08_m.png", "start": 144754193, "end": 144754719}, {"filename": "/GameData/textures/lq_palette/flat_08_n.png", "start": 144754719, "end": 144755246}, {"filename": "/GameData/textures/lq_palette/flat_08_o.png", "start": 144755246, "end": 144755773}, {"filename": "/GameData/textures/lq_palette/flat_08_p.png", "start": 144755773, "end": 144756300}, {"filename": "/GameData/textures/lq_palette/flat_09_a.png", "start": 144756300, "end": 144756827}, {"filename": "/GameData/textures/lq_palette/flat_09_b.png", "start": 144756827, "end": 144757354}, {"filename": "/GameData/textures/lq_palette/flat_09_c.png", "start": 144757354, "end": 144757881}, {"filename": "/GameData/textures/lq_palette/flat_09_d.png", "start": 144757881, "end": 144758407}, {"filename": "/GameData/textures/lq_palette/flat_09_e.png", "start": 144758407, "end": 144758933}, {"filename": "/GameData/textures/lq_palette/flat_09_f.png", "start": 144758933, "end": 144759459}, {"filename": "/GameData/textures/lq_palette/flat_09_g.png", "start": 144759459, "end": 144759985}, {"filename": "/GameData/textures/lq_palette/flat_09_h.png", "start": 144759985, "end": 144760511}, {"filename": "/GameData/textures/lq_palette/flat_09_i.png", "start": 144760511, "end": 144761037}, {"filename": "/GameData/textures/lq_palette/flat_09_j.png", "start": 144761037, "end": 144761563}, {"filename": "/GameData/textures/lq_palette/flat_09_k.png", "start": 144761563, "end": 144762089}, {"filename": "/GameData/textures/lq_palette/flat_09_l.png", "start": 144762089, "end": 144762615}, {"filename": "/GameData/textures/lq_palette/flat_09_m.png", "start": 144762615, "end": 144763141}, {"filename": "/GameData/textures/lq_palette/flat_09_n.png", "start": 144763141, "end": 144763667}, {"filename": "/GameData/textures/lq_palette/flat_09_o.png", "start": 144763667, "end": 144764193}, {"filename": "/GameData/textures/lq_palette/flat_09_p.png", "start": 144764193, "end": 144764719}, {"filename": "/GameData/textures/lq_palette/flat_10_a.png", "start": 144764719, "end": 144765246}, {"filename": "/GameData/textures/lq_palette/flat_10_b.png", "start": 144765246, "end": 144765772}, {"filename": "/GameData/textures/lq_palette/flat_10_c.png", "start": 144765772, "end": 144766298}, {"filename": "/GameData/textures/lq_palette/flat_10_d.png", "start": 144766298, "end": 144766824}, {"filename": "/GameData/textures/lq_palette/flat_10_e.png", "start": 144766824, "end": 144767350}, {"filename": "/GameData/textures/lq_palette/flat_10_f.png", "start": 144767350, "end": 144767876}, {"filename": "/GameData/textures/lq_palette/flat_10_g.png", "start": 144767876, "end": 144768402}, {"filename": "/GameData/textures/lq_palette/flat_10_h.png", "start": 144768402, "end": 144768928}, {"filename": "/GameData/textures/lq_palette/flat_10_i.png", "start": 144768928, "end": 144769454}, {"filename": "/GameData/textures/lq_palette/flat_10_j.png", "start": 144769454, "end": 144769980}, {"filename": "/GameData/textures/lq_palette/flat_10_k.png", "start": 144769980, "end": 144770506}, {"filename": "/GameData/textures/lq_palette/flat_10_l.png", "start": 144770506, "end": 144771032}, {"filename": "/GameData/textures/lq_palette/flat_10_m.png", "start": 144771032, "end": 144771558}, {"filename": "/GameData/textures/lq_palette/flat_10_n.png", "start": 144771558, "end": 144772084}, {"filename": "/GameData/textures/lq_palette/flat_10_o.png", "start": 144772084, "end": 144772610}, {"filename": "/GameData/textures/lq_palette/flat_10_p.png", "start": 144772610, "end": 144773136}, {"filename": "/GameData/textures/lq_palette/flat_11_a.png", "start": 144773136, "end": 144773663}, {"filename": "/GameData/textures/lq_palette/flat_11_b.png", "start": 144773663, "end": 144774190}, {"filename": "/GameData/textures/lq_palette/flat_11_c.png", "start": 144774190, "end": 144774717}, {"filename": "/GameData/textures/lq_palette/flat_11_d.png", "start": 144774717, "end": 144775244}, {"filename": "/GameData/textures/lq_palette/flat_11_e.png", "start": 144775244, "end": 144775771}, {"filename": "/GameData/textures/lq_palette/flat_11_f.png", "start": 144775771, "end": 144776297}, {"filename": "/GameData/textures/lq_palette/flat_11_g.png", "start": 144776297, "end": 144776823}, {"filename": "/GameData/textures/lq_palette/flat_11_h.png", "start": 144776823, "end": 144777349}, {"filename": "/GameData/textures/lq_palette/flat_11_i.png", "start": 144777349, "end": 144777875}, {"filename": "/GameData/textures/lq_palette/flat_11_j.png", "start": 144777875, "end": 144778401}, {"filename": "/GameData/textures/lq_palette/flat_11_k.png", "start": 144778401, "end": 144778927}, {"filename": "/GameData/textures/lq_palette/flat_11_l.png", "start": 144778927, "end": 144779453}, {"filename": "/GameData/textures/lq_palette/flat_11_m.png", "start": 144779453, "end": 144779979}, {"filename": "/GameData/textures/lq_palette/flat_11_n.png", "start": 144779979, "end": 144780505}, {"filename": "/GameData/textures/lq_palette/flat_11_o.png", "start": 144780505, "end": 144781031}, {"filename": "/GameData/textures/lq_palette/flat_11_p.png", "start": 144781031, "end": 144781557}, {"filename": "/GameData/textures/lq_palette/flat_12_a.png", "start": 144781557, "end": 144782083}, {"filename": "/GameData/textures/lq_palette/flat_12_b.png", "start": 144782083, "end": 144782609}, {"filename": "/GameData/textures/lq_palette/flat_12_c.png", "start": 144782609, "end": 144783135}, {"filename": "/GameData/textures/lq_palette/flat_12_d.png", "start": 144783135, "end": 144783661}, {"filename": "/GameData/textures/lq_palette/flat_12_e.png", "start": 144783661, "end": 144784187}, {"filename": "/GameData/textures/lq_palette/flat_12_f.png", "start": 144784187, "end": 144784713}, {"filename": "/GameData/textures/lq_palette/flat_12_g.png", "start": 144784713, "end": 144785239}, {"filename": "/GameData/textures/lq_palette/flat_12_h.png", "start": 144785239, "end": 144785765}, {"filename": "/GameData/textures/lq_palette/flat_12_i.png", "start": 144785765, "end": 144786291}, {"filename": "/GameData/textures/lq_palette/flat_12_j.png", "start": 144786291, "end": 144786817}, {"filename": "/GameData/textures/lq_palette/flat_12_k.png", "start": 144786817, "end": 144787343}, {"filename": "/GameData/textures/lq_palette/flat_12_l.png", "start": 144787343, "end": 144787869}, {"filename": "/GameData/textures/lq_palette/flat_12_m.png", "start": 144787869, "end": 144788395}, {"filename": "/GameData/textures/lq_palette/flat_12_n.png", "start": 144788395, "end": 144788921}, {"filename": "/GameData/textures/lq_palette/flat_12_o.png", "start": 144788921, "end": 144789447}, {"filename": "/GameData/textures/lq_palette/flat_12_p.png", "start": 144789447, "end": 144789973}, {"filename": "/GameData/textures/lq_palette/flat_13_a.png", "start": 144789973, "end": 144790499}, {"filename": "/GameData/textures/lq_palette/flat_13_b.png", "start": 144790499, "end": 144791025}, {"filename": "/GameData/textures/lq_palette/flat_13_c.png", "start": 144791025, "end": 144791551}, {"filename": "/GameData/textures/lq_palette/flat_13_d.png", "start": 144791551, "end": 144792077}, {"filename": "/GameData/textures/lq_palette/flat_13_e.png", "start": 144792077, "end": 144792603}, {"filename": "/GameData/textures/lq_palette/flat_13_f.png", "start": 144792603, "end": 144793129}, {"filename": "/GameData/textures/lq_palette/flat_13_g.png", "start": 144793129, "end": 144793655}, {"filename": "/GameData/textures/lq_palette/flat_13_h.png", "start": 144793655, "end": 144794181}, {"filename": "/GameData/textures/lq_palette/flat_13_i.png", "start": 144794181, "end": 144794707}, {"filename": "/GameData/textures/lq_palette/flat_13_j.png", "start": 144794707, "end": 144795233}, {"filename": "/GameData/textures/lq_palette/flat_13_k.png", "start": 144795233, "end": 144795759}, {"filename": "/GameData/textures/lq_palette/flat_13_l.png", "start": 144795759, "end": 144796285}, {"filename": "/GameData/textures/lq_palette/flat_13_m.png", "start": 144796285, "end": 144796811}, {"filename": "/GameData/textures/lq_palette/flat_13_n.png", "start": 144796811, "end": 144797337}, {"filename": "/GameData/textures/lq_palette/flat_13_o.png", "start": 144797337, "end": 144797863}, {"filename": "/GameData/textures/lq_palette/flat_13_p.png", "start": 144797863, "end": 144798389}, {"filename": "/GameData/textures/lq_palette/flat_14_a.png", "start": 144798389, "end": 144798937}, {"filename": "/GameData/textures/lq_palette/flat_14_b.png", "start": 144798937, "end": 144799463}, {"filename": "/GameData/textures/lq_palette/flat_14_c.png", "start": 144799463, "end": 144799989}, {"filename": "/GameData/textures/lq_palette/flat_14_d.png", "start": 144799989, "end": 144800515}, {"filename": "/GameData/textures/lq_palette/flat_14_e.png", "start": 144800515, "end": 144801041}, {"filename": "/GameData/textures/lq_palette/flat_14_f.png", "start": 144801041, "end": 144801567}, {"filename": "/GameData/textures/lq_palette/flat_14_g.png", "start": 144801567, "end": 144802093}, {"filename": "/GameData/textures/lq_palette/flat_14_h.png", "start": 144802093, "end": 144802619}, {"filename": "/GameData/textures/lq_palette/flat_14_i.png", "start": 144802619, "end": 144803145}, {"filename": "/GameData/textures/lq_palette/flat_14_j.png", "start": 144803145, "end": 144803671}, {"filename": "/GameData/textures/lq_palette/flat_14_k.png", "start": 144803671, "end": 144804197}, {"filename": "/GameData/textures/lq_palette/flat_14_l.png", "start": 144804197, "end": 144804723}, {"filename": "/GameData/textures/lq_palette/flat_14_m.png", "start": 144804723, "end": 144805249}, {"filename": "/GameData/textures/lq_palette/flat_14_n.png", "start": 144805249, "end": 144805775}, {"filename": "/GameData/textures/lq_palette/flat_14_o.png", "start": 144805775, "end": 144806301}, {"filename": "/GameData/textures/lq_palette/flat_14_p.png", "start": 144806301, "end": 144806827}, {"filename": "/GameData/textures/lq_palette/flat_15_a_fbr.png", "start": 144806827, "end": 144807353}, {"filename": "/GameData/textures/lq_palette/flat_15_b_fbr.png", "start": 144807353, "end": 144807879}, {"filename": "/GameData/textures/lq_palette/flat_15_c_fbr.png", "start": 144807879, "end": 144808405}, {"filename": "/GameData/textures/lq_palette/flat_15_d_fbr.png", "start": 144808405, "end": 144808931}, {"filename": "/GameData/textures/lq_palette/flat_15_e_fbr.png", "start": 144808931, "end": 144809457}, {"filename": "/GameData/textures/lq_palette/flat_15_f_fbr.png", "start": 144809457, "end": 144809983}, {"filename": "/GameData/textures/lq_palette/flat_15_g_fbr.png", "start": 144809983, "end": 144810509}, {"filename": "/GameData/textures/lq_palette/flat_15_h_fbr.png", "start": 144810509, "end": 144811035}, {"filename": "/GameData/textures/lq_palette/flat_15_i_fbr.png", "start": 144811035, "end": 144811561}, {"filename": "/GameData/textures/lq_palette/flat_15_j_fbr.png", "start": 144811561, "end": 144812087}, {"filename": "/GameData/textures/lq_palette/flat_15_k_fbr.png", "start": 144812087, "end": 144812613}, {"filename": "/GameData/textures/lq_palette/flat_15_l_fbr.png", "start": 144812613, "end": 144813139}, {"filename": "/GameData/textures/lq_palette/flat_15_m_fbr.png", "start": 144813139, "end": 144813665}, {"filename": "/GameData/textures/lq_palette/flat_15_n_fbr.png", "start": 144813665, "end": 144814191}, {"filename": "/GameData/textures/lq_palette/flat_15_o_fbr.png", "start": 144814191, "end": 144814718}, {"filename": "/GameData/textures/lq_palette/flat_15_p_fbr.png", "start": 144814718, "end": 144815245}, {"filename": "/GameData/textures/lq_palette/flat_16_a_fbr.png", "start": 144815245, "end": 144815771}, {"filename": "/GameData/textures/lq_palette/flat_16_b_fbr.png", "start": 144815771, "end": 144816297}, {"filename": "/GameData/textures/lq_palette/flat_16_c_fbr.png", "start": 144816297, "end": 144816823}, {"filename": "/GameData/textures/lq_palette/flat_16_d_fbr.png", "start": 144816823, "end": 144817349}, {"filename": "/GameData/textures/lq_palette/flat_16_e_fbr.png", "start": 144817349, "end": 144817876}, {"filename": "/GameData/textures/lq_palette/flat_16_f_fbr.png", "start": 144817876, "end": 144818403}, {"filename": "/GameData/textures/lq_palette/flat_16_g_fbr.png", "start": 144818403, "end": 144818928}, {"filename": "/GameData/textures/lq_palette/flat_16_h_fbr.png", "start": 144818928, "end": 144819454}, {"filename": "/GameData/textures/lq_palette/flat_16_i_fbr.png", "start": 144819454, "end": 144819980}, {"filename": "/GameData/textures/lq_palette/flat_16_j_fbr.png", "start": 144819980, "end": 144820506}, {"filename": "/GameData/textures/lq_palette/flat_16_k_fbr.png", "start": 144820506, "end": 144821032}, {"filename": "/GameData/textures/lq_palette/flat_16_l_fbr.png", "start": 144821032, "end": 144821558}, {"filename": "/GameData/textures/lq_palette/flat_16_m_fbr.png", "start": 144821558, "end": 144822085}, {"filename": "/GameData/textures/lq_palette/flat_16_n_fbr.png", "start": 144822085, "end": 144822612}, {"filename": "/GameData/textures/lq_palette/flat_16_o_fbr.png", "start": 144822612, "end": 144823137}, {"filename": "/GameData/textures/lq_palette/flat_16_p_fbr.png", "start": 144823137, "end": 144823663}, {"filename": "/GameData/textures/lq_props/JarBod1.png", "start": 144823663, "end": 144824010}, {"filename": "/GameData/textures/lq_props/JarBod2.png", "start": 144824010, "end": 144824328}, {"filename": "/GameData/textures/lq_props/JarTop1.png", "start": 144824328, "end": 144824553}, {"filename": "/GameData/textures/lq_props/JarTop2.png", "start": 144824553, "end": 144824693}, {"filename": "/GameData/textures/lq_props/crate-door-brn.png", "start": 144824693, "end": 144833331}, {"filename": "/GameData/textures/lq_props/crate-door-grn.png", "start": 144833331, "end": 144843387}, {"filename": "/GameData/textures/lq_props/crate-door-orn.png", "start": 144843387, "end": 144852486}, {"filename": "/GameData/textures/lq_props/crate-side-brn.png", "start": 144852486, "end": 144872612}, {"filename": "/GameData/textures/lq_props/crate-side-grn.png", "start": 144872612, "end": 144895525}, {"filename": "/GameData/textures/lq_props/crate-side-orn.png", "start": 144895525, "end": 144916854}, {"filename": "/GameData/textures/lq_props/go-ep0_fbr.png", "start": 144916854, "end": 144918629}, {"filename": "/GameData/textures/lq_props/med_book_blue.png", "start": 144918629, "end": 144919209}, {"filename": "/GameData/textures/lq_props/med_book_green.png", "start": 144919209, "end": 144919832}, {"filename": "/GameData/textures/lq_props/med_book_pink.png", "start": 144919832, "end": 144920487}, {"filename": "/GameData/textures/lq_props/med_book_red.png", "start": 144920487, "end": 144921204}, {"filename": "/GameData/textures/lq_props/med_book_teal.png", "start": 144921204, "end": 144921791}, {"filename": "/GameData/textures/lq_props/med_books_wood.png", "start": 144921791, "end": 144934721}, {"filename": "/GameData/textures/lq_props/med_dbrick4_p1.png", "start": 144934721, "end": 144966558}, {"filename": "/GameData/textures/lq_props/med_dbrick4_p2.png", "start": 144966558, "end": 144999979}, {"filename": "/GameData/textures/lq_props/med_ebrick9_p1.png", "start": 144999979, "end": 145031870}, {"filename": "/GameData/textures/lq_props/med_ebrick9_p2.png", "start": 145031870, "end": 145063236}, {"filename": "/GameData/textures/lq_props/note-e0_fbr.png", "start": 145063236, "end": 145082665}, {"filename": "/GameData/textures/lq_props/plus_0blink_fbr.png", "start": 145082665, "end": 145082894}, {"filename": "/GameData/textures/lq_props/plus_0tvnoise.png", "start": 145082894, "end": 145083880}, {"filename": "/GameData/textures/lq_props/plus_1blink_fbr.png", "start": 145083880, "end": 145084109}, {"filename": "/GameData/textures/lq_props/plus_1tvnoise.png", "start": 145084109, "end": 145085087}, {"filename": "/GameData/textures/lq_props/plus_2blink_fbr.png", "start": 145085087, "end": 145085318}, {"filename": "/GameData/textures/lq_props/plus_2tvnoise.png", "start": 145085318, "end": 145086291}, {"filename": "/GameData/textures/lq_props/plus_3blink_fbr.png", "start": 145086291, "end": 145086522}, {"filename": "/GameData/textures/lq_props/plus_3tvnoise.png", "start": 145086522, "end": 145087504}, {"filename": "/GameData/textures/lq_props/plus_4blink_fbr.png", "start": 145087504, "end": 145087734}, {"filename": "/GameData/textures/lq_props/plus_4tvnoise.png", "start": 145087734, "end": 145088718}, {"filename": "/GameData/textures/lq_props/plus_5tvnoise.png", "start": 145088718, "end": 145089682}, {"filename": "/GameData/textures/lq_props/plus_6tvnoise.png", "start": 145089682, "end": 145090672}, {"filename": "/GameData/textures/lq_props/plus_7tvnoise.png", "start": 145090672, "end": 145091646}, {"filename": "/GameData/textures/lq_props/plus_8tvnoise.png", "start": 145091646, "end": 145092647}, {"filename": "/GameData/textures/lq_props/plus_9tvnoise.png", "start": 145092647, "end": 145093624}, {"filename": "/GameData/textures/lq_props/plus_ablink_fbr.png", "start": 145093624, "end": 145093854}, {"filename": "/GameData/textures/lq_props/plus_atvnoise.png", "start": 145093854, "end": 145094414}, {"filename": "/GameData/textures/lq_props/plus_atvnoise64.png", "start": 145094414, "end": 145095684}, {"filename": "/GameData/textures/lq_props/qr.png", "start": 145095684, "end": 145097104}, {"filename": "/GameData/textures/lq_props/radio16.png", "start": 145097104, "end": 145097872}, {"filename": "/GameData/textures/lq_props/radio32.png", "start": 145097872, "end": 145098862}, {"filename": "/GameData/textures/lq_props/radio64.png", "start": 145098862, "end": 145100620}, {"filename": "/GameData/textures/lq_props/radiowood.png", "start": 145100620, "end": 145102763}, {"filename": "/GameData/textures/lq_props/secret_gem_1.png", "start": 145102763, "end": 145105799}, {"filename": "/GameData/textures/lq_props/secret_gem_2.png", "start": 145105799, "end": 145108503}, {"filename": "/GameData/textures/lq_props/secret_gem_3.png", "start": 145108503, "end": 145111563}, {"filename": "/GameData/textures/lq_props/secret_gem_4.png", "start": 145111563, "end": 145115321}, {"filename": "/GameData/textures/lq_props/secret_gem_h.png", "start": 145115321, "end": 145115872}, {"filename": "/GameData/textures/lq_tech/_t_fence01_fbr.png", "start": 145115872, "end": 145121589}, {"filename": "/GameData/textures/lq_tech/_t_flare01_fbr.png", "start": 145121589, "end": 145121975}, {"filename": "/GameData/textures/lq_tech/aqconc03.png", "start": 145121975, "end": 145132535}, {"filename": "/GameData/textures/lq_tech/aqconc04.png", "start": 145132535, "end": 145143591}, {"filename": "/GameData/textures/lq_tech/aqconc05.png", "start": 145143591, "end": 145148182}, {"filename": "/GameData/textures/lq_tech/aqf006b.png", "start": 145148182, "end": 145150809}, {"filename": "/GameData/textures/lq_tech/aqf032.png", "start": 145150809, "end": 145152080}, {"filename": "/GameData/textures/lq_tech/aqf049.png", "start": 145152080, "end": 145154357}, {"filename": "/GameData/textures/lq_tech/aqf074.png", "start": 145154357, "end": 145157779}, {"filename": "/GameData/textures/lq_tech/aqf075.png", "start": 145157779, "end": 145160938}, {"filename": "/GameData/textures/lq_tech/aqmetl01.png", "start": 145160938, "end": 145163666}, {"filename": "/GameData/textures/lq_tech/aqmetl07.png", "start": 145163666, "end": 145168437}, {"filename": "/GameData/textures/lq_tech/aqmetl14.png", "start": 145168437, "end": 145176169}, {"filename": "/GameData/textures/lq_tech/aqmetl28.png", "start": 145176169, "end": 145177821}, {"filename": "/GameData/textures/lq_tech/aqmetl30.png", "start": 145177821, "end": 145179473}, {"filename": "/GameData/textures/lq_tech/aqmetl33.png", "start": 145179473, "end": 145181909}, {"filename": "/GameData/textures/lq_tech/aqpanl09.png", "start": 145181909, "end": 145184701}, {"filename": "/GameData/textures/lq_tech/aqpanl10.png", "start": 145184701, "end": 145189595}, {"filename": "/GameData/textures/lq_tech/aqpipe01.png", "start": 145189595, "end": 145193783}, {"filename": "/GameData/textures/lq_tech/aqpipe04.png", "start": 145193783, "end": 145196247}, {"filename": "/GameData/textures/lq_tech/aqpipe05.png", "start": 145196247, "end": 145199413}, {"filename": "/GameData/textures/lq_tech/aqpipe08.png", "start": 145199413, "end": 145205045}, {"filename": "/GameData/textures/lq_tech/aqpipe09.png", "start": 145205045, "end": 145213149}, {"filename": "/GameData/textures/lq_tech/aqpipe12.png", "start": 145213149, "end": 145221269}, {"filename": "/GameData/textures/lq_tech/aqpipe13.png", "start": 145221269, "end": 145227580}, {"filename": "/GameData/textures/lq_tech/aqpipe14.png", "start": 145227580, "end": 145234933}, {"filename": "/GameData/textures/lq_tech/aqrust01.png", "start": 145234933, "end": 145237564}, {"filename": "/GameData/textures/lq_tech/aqrust02.png", "start": 145237564, "end": 145242865}, {"filename": "/GameData/textures/lq_tech/aqrust03.png", "start": 145242865, "end": 145248168}, {"filename": "/GameData/textures/lq_tech/aqrust03b.png", "start": 145248168, "end": 145250898}, {"filename": "/GameData/textures/lq_tech/aqrust04.png", "start": 145250898, "end": 145252291}, {"filename": "/GameData/textures/lq_tech/aqrust09.png", "start": 145252291, "end": 145256179}, {"filename": "/GameData/textures/lq_tech/aqrust10.png", "start": 145256179, "end": 145259345}, {"filename": "/GameData/textures/lq_tech/aqsect14.png", "start": 145259345, "end": 145261721}, {"filename": "/GameData/textures/lq_tech/aqsect15.png", "start": 145261721, "end": 145264668}, {"filename": "/GameData/textures/lq_tech/aqsect16.png", "start": 145264668, "end": 145267914}, {"filename": "/GameData/textures/lq_tech/aqsect16b.png", "start": 145267914, "end": 145272871}, {"filename": "/GameData/textures/lq_tech/aqsupp01.png", "start": 145272871, "end": 145275517}, {"filename": "/GameData/textures/lq_tech/aqsupp02.png", "start": 145275517, "end": 145280382}, {"filename": "/GameData/textures/lq_tech/aqsupp03.png", "start": 145280382, "end": 145284284}, {"filename": "/GameData/textures/lq_tech/aqsupp04.png", "start": 145284284, "end": 145285827}, {"filename": "/GameData/textures/lq_tech/aqsupp06.png", "start": 145285827, "end": 145287206}, {"filename": "/GameData/textures/lq_tech/aqsupp07.png", "start": 145287206, "end": 145288561}, {"filename": "/GameData/textures/lq_tech/aqsupp08.png", "start": 145288561, "end": 145290763}, {"filename": "/GameData/textures/lq_tech/aqsupp09.png", "start": 145290763, "end": 145293616}, {"filename": "/GameData/textures/lq_tech/aqtrim01.png", "start": 145293616, "end": 145294843}, {"filename": "/GameData/textures/lq_tech/aqtrim02.png", "start": 145294843, "end": 145295767}, {"filename": "/GameData/textures/lq_tech/aqtrim03.png", "start": 145295767, "end": 145296338}, {"filename": "/GameData/textures/lq_tech/aqtrim08.png", "start": 145296338, "end": 145297070}, {"filename": "/GameData/textures/lq_tech/butmet.png", "start": 145297070, "end": 145298981}, {"filename": "/GameData/textures/lq_tech/comp1_1.png", "start": 145298981, "end": 145302337}, {"filename": "/GameData/textures/lq_tech/comp1_2.png", "start": 145302337, "end": 145305016}, {"filename": "/GameData/textures/lq_tech/comp1_3.png", "start": 145305016, "end": 145307505}, {"filename": "/GameData/textures/lq_tech/comp1_3b.png", "start": 145307505, "end": 145310133}, {"filename": "/GameData/textures/lq_tech/comp1_4.png", "start": 145310133, "end": 145311738}, {"filename": "/GameData/textures/lq_tech/comp1_5.png", "start": 145311738, "end": 145314837}, {"filename": "/GameData/textures/lq_tech/comp1_6.png", "start": 145314837, "end": 145318279}, {"filename": "/GameData/textures/lq_tech/comp1_7.png", "start": 145318279, "end": 145320624}, {"filename": "/GameData/textures/lq_tech/comp1_8.png", "start": 145320624, "end": 145322953}, {"filename": "/GameData/textures/lq_tech/compbase.png", "start": 145322953, "end": 145325372}, {"filename": "/GameData/textures/lq_tech/crate.png", "start": 145325372, "end": 145328489}, {"filename": "/GameData/textures/lq_tech/crate0_bottom.png", "start": 145328489, "end": 145330605}, {"filename": "/GameData/textures/lq_tech/crate0_s_bottom.png", "start": 145330605, "end": 145331693}, {"filename": "/GameData/textures/lq_tech/crate0_s_sside.png", "start": 145331693, "end": 145332638}, {"filename": "/GameData/textures/lq_tech/crate0_s_top.png", "start": 145332638, "end": 145334077}, {"filename": "/GameData/textures/lq_tech/crate0_s_tside.png", "start": 145334077, "end": 145335740}, {"filename": "/GameData/textures/lq_tech/crate0_side.png", "start": 145335740, "end": 145339208}, {"filename": "/GameData/textures/lq_tech/crate0_top.png", "start": 145339208, "end": 145341954}, {"filename": "/GameData/textures/lq_tech/crate0_xs_bot.png", "start": 145341954, "end": 145342621}, {"filename": "/GameData/textures/lq_tech/crate0_xs_sside.png", "start": 145342621, "end": 145343566}, {"filename": "/GameData/textures/lq_tech/crate0_xs_top.png", "start": 145343566, "end": 145344436}, {"filename": "/GameData/textures/lq_tech/crate0_xs_tside.png", "start": 145344436, "end": 145345376}, {"filename": "/GameData/textures/lq_tech/crate1_bottom.png", "start": 145345376, "end": 145347340}, {"filename": "/GameData/textures/lq_tech/crate1_s_bottom.png", "start": 145347340, "end": 145348370}, {"filename": "/GameData/textures/lq_tech/crate1_s_sside.png", "start": 145348370, "end": 145349248}, {"filename": "/GameData/textures/lq_tech/crate1_s_top.png", "start": 145349248, "end": 145350381}, {"filename": "/GameData/textures/lq_tech/crate1_s_tside.png", "start": 145350381, "end": 145351846}, {"filename": "/GameData/textures/lq_tech/crate1_side.png", "start": 145351846, "end": 145354963}, {"filename": "/GameData/textures/lq_tech/crate1_top.png", "start": 145354963, "end": 145357084}, {"filename": "/GameData/textures/lq_tech/crate1_xs_bot.png", "start": 145357084, "end": 145357738}, {"filename": "/GameData/textures/lq_tech/crate1_xs_sside.png", "start": 145357738, "end": 145358616}, {"filename": "/GameData/textures/lq_tech/crate1_xs_top.png", "start": 145358616, "end": 145359323}, {"filename": "/GameData/textures/lq_tech/crate1_xs_tside.png", "start": 145359323, "end": 145360161}, {"filename": "/GameData/textures/lq_tech/dem4_1.png", "start": 145360161, "end": 145369560}, {"filename": "/GameData/textures/lq_tech/dem4_4.png", "start": 145369560, "end": 145377606}, {"filename": "/GameData/textures/lq_tech/dem5_3_fbr.png", "start": 145377606, "end": 145386423}, {"filename": "/GameData/textures/lq_tech/door02_1.png", "start": 145386423, "end": 145390555}, {"filename": "/GameData/textures/lq_tech/doorr02_1.png", "start": 145390555, "end": 145392121}, {"filename": "/GameData/textures/lq_tech/doortrak1.png", "start": 145392121, "end": 145393732}, {"filename": "/GameData/textures/lq_tech/doortrak2-corn.png", "start": 145393732, "end": 145395029}, {"filename": "/GameData/textures/lq_tech/doortrak2.png", "start": 145395029, "end": 145396694}, {"filename": "/GameData/textures/lq_tech/ecop1_1.png", "start": 145396694, "end": 145399771}, {"filename": "/GameData/textures/lq_tech/ecop1_4.png", "start": 145399771, "end": 145403372}, {"filename": "/GameData/textures/lq_tech/edoor01_1.png", "start": 145403372, "end": 145415518}, {"filename": "/GameData/textures/lq_tech/edoor02.png", "start": 145415518, "end": 145427569}, {"filename": "/GameData/textures/lq_tech/edoor02.png.png", "start": 145427569, "end": 145439620}, {"filename": "/GameData/textures/lq_tech/fddoor01.png", "start": 145439620, "end": 145448374}, {"filename": "/GameData/textures/lq_tech/fddoor01b.png", "start": 145448374, "end": 145459057}, {"filename": "/GameData/textures/lq_tech/fdoor02.png", "start": 145459057, "end": 145467611}, {"filename": "/GameData/textures/lq_tech/flat4.png", "start": 145467611, "end": 145468784}, {"filename": "/GameData/textures/lq_tech/floor5_2.png", "start": 145468784, "end": 145471510}, {"filename": "/GameData/textures/lq_tech/floor5_3.png", "start": 145471510, "end": 145473690}, {"filename": "/GameData/textures/lq_tech/laserfield1_fbr.png", "start": 145473690, "end": 145484699}, {"filename": "/GameData/textures/lq_tech/light2.png", "start": 145484699, "end": 145485297}, {"filename": "/GameData/textures/lq_tech/lit8nb.png", "start": 145485297, "end": 145485611}, {"filename": "/GameData/textures/lq_tech/lit8sfb_fbr.png", "start": 145485611, "end": 145485908}, {"filename": "/GameData/textures/lq_tech/met2.png", "start": 145485908, "end": 145498232}, {"filename": "/GameData/textures/lq_tech/metalstrip_1.png", "start": 145498232, "end": 145500515}, {"filename": "/GameData/textures/lq_tech/plat_side1.png", "start": 145500515, "end": 145501280}, {"filename": "/GameData/textures/lq_tech/plat_stem.png", "start": 145501280, "end": 145501926}, {"filename": "/GameData/textures/lq_tech/plat_top1.png", "start": 145501926, "end": 145505326}, {"filename": "/GameData/textures/lq_tech/plat_top2.png", "start": 145505326, "end": 145509041}, {"filename": "/GameData/textures/lq_tech/plat_top3.png", "start": 145509041, "end": 145512449}, {"filename": "/GameData/textures/lq_tech/plat_top4.png", "start": 145512449, "end": 145515145}, {"filename": "/GameData/textures/lq_tech/plat_top5.png", "start": 145515145, "end": 145517240}, {"filename": "/GameData/textures/lq_tech/plus_0_gkey.png", "start": 145517240, "end": 145518304}, {"filename": "/GameData/textures/lq_tech/plus_0_skey.png", "start": 145518304, "end": 145519320}, {"filename": "/GameData/textures/lq_tech/plus_0_tscrn0.png", "start": 145519320, "end": 145521433}, {"filename": "/GameData/textures/lq_tech/plus_0_tscrn1.png", "start": 145521433, "end": 145523558}, {"filename": "/GameData/textures/lq_tech/plus_0arrow2_d_fbr.png", "start": 145523558, "end": 145524812}, {"filename": "/GameData/textures/lq_tech/plus_0arrow2_h_fbr.png", "start": 145524812, "end": 145526081}, {"filename": "/GameData/textures/lq_tech/plus_0arrow2_u_fbr.png", "start": 145526081, "end": 145527339}, {"filename": "/GameData/textures/lq_tech/plus_0arrow_d_fbr.png", "start": 145527339, "end": 145528221}, {"filename": "/GameData/textures/lq_tech/plus_0arrow_h_fbr.png", "start": 145528221, "end": 145529100}, {"filename": "/GameData/textures/lq_tech/plus_0arrow_u_fbr.png", "start": 145529100, "end": 145529975}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn1b_fbr.png", "start": 145529975, "end": 145530864}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn2_fbr.png", "start": 145530864, "end": 145531210}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn2b_fbr.png", "start": 145531210, "end": 145531550}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn_fbr.png", "start": 145531550, "end": 145532831}, {"filename": "/GameData/textures/lq_tech/plus_0button3_fbr.png", "start": 145532831, "end": 145534707}, {"filename": "/GameData/textures/lq_tech/plus_0lit8s.png", "start": 145534707, "end": 145535021}, {"filename": "/GameData/textures/lq_tech/plus_0planet_a_fbr.png", "start": 145535021, "end": 145536612}, {"filename": "/GameData/textures/lq_tech/plus_0planet_b_fbr.png", "start": 145536612, "end": 145537722}, {"filename": "/GameData/textures/lq_tech/plus_0planet_c_fbr.png", "start": 145537722, "end": 145538822}, {"filename": "/GameData/textures/lq_tech/plus_0slipbot.png", "start": 145538822, "end": 145542105}, {"filename": "/GameData/textures/lq_tech/plus_0sliptop.png", "start": 145542105, "end": 145545748}, {"filename": "/GameData/textures/lq_tech/plus_0tek_jump1_fbr.png", "start": 145545748, "end": 145548231}, {"filename": "/GameData/textures/lq_tech/plus_0term128.png", "start": 145548231, "end": 145551035}, {"filename": "/GameData/textures/lq_tech/plus_0term64.png", "start": 145551035, "end": 145552227}, {"filename": "/GameData/textures/lq_tech/plus_0tlight1.png", "start": 145552227, "end": 145552825}, {"filename": "/GameData/textures/lq_tech/plus_0tlight2.png", "start": 145552825, "end": 145553438}, {"filename": "/GameData/textures/lq_tech/plus_0tlight3.png", "start": 145553438, "end": 145554009}, {"filename": "/GameData/textures/lq_tech/plus_1_gkey.png", "start": 145554009, "end": 145555059}, {"filename": "/GameData/textures/lq_tech/plus_1_skey.png", "start": 145555059, "end": 145556064}, {"filename": "/GameData/textures/lq_tech/plus_1arrow2_d_fbr.png", "start": 145556064, "end": 145557320}, {"filename": "/GameData/textures/lq_tech/plus_1arrow2_h_fbr.png", "start": 145557320, "end": 145558587}, {"filename": "/GameData/textures/lq_tech/plus_1arrow2_u_fbr.png", "start": 145558587, "end": 145559845}, {"filename": "/GameData/textures/lq_tech/plus_1arrow_d_fbr.png", "start": 145559845, "end": 145560731}, {"filename": "/GameData/textures/lq_tech/plus_1arrow_h_fbr.png", "start": 145560731, "end": 145561613}, {"filename": "/GameData/textures/lq_tech/plus_1arrow_u_fbr.png", "start": 145561613, "end": 145562491}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn.png", "start": 145562491, "end": 145563770}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn1b.png", "start": 145563770, "end": 145564648}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn2.png", "start": 145564648, "end": 145564998}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn2b.png", "start": 145564998, "end": 145565356}, {"filename": "/GameData/textures/lq_tech/plus_1planet_a_fbr.png", "start": 145565356, "end": 145566977}, {"filename": "/GameData/textures/lq_tech/plus_1planet_b_fbr.png", "start": 145566977, "end": 145568062}, {"filename": "/GameData/textures/lq_tech/plus_1planet_c_fbr.png", "start": 145568062, "end": 145569134}, {"filename": "/GameData/textures/lq_tech/plus_1tek_jump1_fbr.png", "start": 145569134, "end": 145571617}, {"filename": "/GameData/textures/lq_tech/plus_1term128.png", "start": 145571617, "end": 145574420}, {"filename": "/GameData/textures/lq_tech/plus_1term64.png", "start": 145574420, "end": 145575612}, {"filename": "/GameData/textures/lq_tech/plus_2_gkey.png", "start": 145575612, "end": 145576674}, {"filename": "/GameData/textures/lq_tech/plus_2_skey.png", "start": 145576674, "end": 145577680}, {"filename": "/GameData/textures/lq_tech/plus_2arrow2_d_fbr.png", "start": 145577680, "end": 145578923}, {"filename": "/GameData/textures/lq_tech/plus_2arrow2_h_fbr.png", "start": 145578923, "end": 145580184}, {"filename": "/GameData/textures/lq_tech/plus_2arrow2_u_fbr.png", "start": 145580184, "end": 145581430}, {"filename": "/GameData/textures/lq_tech/plus_2arrow_d_fbr.png", "start": 145581430, "end": 145582328}, {"filename": "/GameData/textures/lq_tech/plus_2arrow_h_fbr.png", "start": 145582328, "end": 145583218}, {"filename": "/GameData/textures/lq_tech/plus_2arrow_u_fbr.png", "start": 145583218, "end": 145584105}, {"filename": "/GameData/textures/lq_tech/plus_2planet_a_fbr.png", "start": 145584105, "end": 145585708}, {"filename": "/GameData/textures/lq_tech/plus_2planet_b_fbr.png", "start": 145585708, "end": 145586810}, {"filename": "/GameData/textures/lq_tech/plus_2planet_c_fbr.png", "start": 145586810, "end": 145587928}, {"filename": "/GameData/textures/lq_tech/plus_3planet_a_fbr.png", "start": 145587928, "end": 145589518}, {"filename": "/GameData/textures/lq_tech/plus_3planet_b_fbr.png", "start": 145589518, "end": 145590612}, {"filename": "/GameData/textures/lq_tech/plus_3planet_c_fbr.png", "start": 145590612, "end": 145591735}, {"filename": "/GameData/textures/lq_tech/plus_4planet_a_fbr.png", "start": 145591735, "end": 145593344}, {"filename": "/GameData/textures/lq_tech/plus_4planet_b_fbr.png", "start": 145593344, "end": 145594459}, {"filename": "/GameData/textures/lq_tech/plus_4planet_c_fbr.png", "start": 145594459, "end": 145595606}, {"filename": "/GameData/textures/lq_tech/plus_5planet_a_fbr.png", "start": 145595606, "end": 145597238}, {"filename": "/GameData/textures/lq_tech/plus_5planet_b_fbr.png", "start": 145597238, "end": 145598365}, {"filename": "/GameData/textures/lq_tech/plus_5planet_c_fbr.png", "start": 145598365, "end": 145599472}, {"filename": "/GameData/textures/lq_tech/plus_6planet_a_fbr.png", "start": 145599472, "end": 145601093}, {"filename": "/GameData/textures/lq_tech/plus_6planet_b_fbr.png", "start": 145601093, "end": 145602217}, {"filename": "/GameData/textures/lq_tech/plus_6planet_c_fbr.png", "start": 145602217, "end": 145603311}, {"filename": "/GameData/textures/lq_tech/plus_7planet_a_fbr.png", "start": 145603311, "end": 145604899}, {"filename": "/GameData/textures/lq_tech/plus_7planet_b_fbr.png", "start": 145604899, "end": 145606012}, {"filename": "/GameData/textures/lq_tech/plus_7planet_c_fbr.png", "start": 145606012, "end": 145607104}, {"filename": "/GameData/textures/lq_tech/plus_8planet_a_fbr.png", "start": 145607104, "end": 145608726}, {"filename": "/GameData/textures/lq_tech/plus_8planet_b_fbr.png", "start": 145608726, "end": 145609853}, {"filename": "/GameData/textures/lq_tech/plus_8planet_c_fbr.png", "start": 145609853, "end": 145610942}, {"filename": "/GameData/textures/lq_tech/plus_9planet_a_fbr.png", "start": 145610942, "end": 145612526}, {"filename": "/GameData/textures/lq_tech/plus_9planet_b_fbr.png", "start": 145612526, "end": 145613614}, {"filename": "/GameData/textures/lq_tech/plus_9planet_c_fbr.png", "start": 145613614, "end": 145614690}, {"filename": "/GameData/textures/lq_tech/plus_A_tscrn0.png", "start": 145614690, "end": 145616313}, {"filename": "/GameData/textures/lq_tech/plus_A_tscrn1.png", "start": 145616313, "end": 145618822}, {"filename": "/GameData/textures/lq_tech/plus_A_tscrn2.png", "start": 145618822, "end": 145620422}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn.png", "start": 145620422, "end": 145621711}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn1b.png", "start": 145621711, "end": 145623000}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn2.png", "start": 145623000, "end": 145623371}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn2b.png", "start": 145623371, "end": 145623742}, {"filename": "/GameData/textures/lq_tech/plus_abasebtnb.png", "start": 145623742, "end": 145624113}, {"filename": "/GameData/textures/lq_tech/plus_abutton3_fbr.png", "start": 145624113, "end": 145626001}, {"filename": "/GameData/textures/lq_tech/plus_alit8s_fbr.png", "start": 145626001, "end": 145626298}, {"filename": "/GameData/textures/lq_tech/plus_atek_jump1_fbr.png", "start": 145626298, "end": 145628791}, {"filename": "/GameData/textures/lq_tech/plus_atlight1_fbr.png", "start": 145628791, "end": 145629381}, {"filename": "/GameData/textures/lq_tech/plus_atlight2_fbr.png", "start": 145629381, "end": 145629954}, {"filename": "/GameData/textures/lq_tech/plus_atlight3_fbr.png", "start": 145629954, "end": 145630556}, {"filename": "/GameData/textures/lq_tech/rw33_1.png", "start": 145630556, "end": 145636121}, {"filename": "/GameData/textures/lq_tech/rw33_2.png", "start": 145636121, "end": 145641975}, {"filename": "/GameData/textures/lq_tech/rw33_3.png", "start": 145641975, "end": 145647721}, {"filename": "/GameData/textures/lq_tech/rw33_4.png", "start": 145647721, "end": 145650287}, {"filename": "/GameData/textures/lq_tech/rw33_4b_l.png", "start": 145650287, "end": 145653247}, {"filename": "/GameData/textures/lq_tech/rw33_5.png", "start": 145653247, "end": 145656119}, {"filename": "/GameData/textures/lq_tech/rw33_flat.png", "start": 145656119, "end": 145662070}, {"filename": "/GameData/textures/lq_tech/rw33_lit.png", "start": 145662070, "end": 145662894}, {"filename": "/GameData/textures/lq_tech/rw33b_1.png", "start": 145662894, "end": 145668357}, {"filename": "/GameData/textures/lq_tech/rw33b_2.png", "start": 145668357, "end": 145674199}, {"filename": "/GameData/textures/lq_tech/rw33b_3.png", "start": 145674199, "end": 145680186}, {"filename": "/GameData/textures/lq_tech/rw33b_4.png", "start": 145680186, "end": 145682948}, {"filename": "/GameData/textures/lq_tech/rw33b_5.png", "start": 145682948, "end": 145685997}, {"filename": "/GameData/textures/lq_tech/rw33b_flat.png", "start": 145685997, "end": 145692195}, {"filename": "/GameData/textures/lq_tech/rw33b_lit.png", "start": 145692195, "end": 145693147}, {"filename": "/GameData/textures/lq_tech/rw37_1.png", "start": 145693147, "end": 145698759}, {"filename": "/GameData/textures/lq_tech/rw37_2.png", "start": 145698759, "end": 145704889}, {"filename": "/GameData/textures/lq_tech/rw37_3.png", "start": 145704889, "end": 145710784}, {"filename": "/GameData/textures/lq_tech/rw37_4.png", "start": 145710784, "end": 145717795}, {"filename": "/GameData/textures/lq_tech/rw37_trim1.png", "start": 145717795, "end": 145720303}, {"filename": "/GameData/textures/lq_tech/rw37_trim2.png", "start": 145720303, "end": 145722086}, {"filename": "/GameData/textures/lq_tech/rw37_trim3.png", "start": 145722086, "end": 145724577}, {"filename": "/GameData/textures/lq_tech/rw39_1_fbr.png", "start": 145724577, "end": 145730522}, {"filename": "/GameData/textures/lq_tech/spotlight_fbr.png", "start": 145730522, "end": 145733134}, {"filename": "/GameData/textures/lq_tech/star_lasergrid.png", "start": 145733134, "end": 145733514}, {"filename": "/GameData/textures/lq_tech/t_band1a.png", "start": 145733514, "end": 145736018}, {"filename": "/GameData/textures/lq_tech/t_band1b.png", "start": 145736018, "end": 145738626}, {"filename": "/GameData/textures/lq_tech/t_blok01.png", "start": 145738626, "end": 145741304}, {"filename": "/GameData/textures/lq_tech/t_blok01a.png", "start": 145741304, "end": 145744076}, {"filename": "/GameData/textures/lq_tech/t_blok02.png", "start": 145744076, "end": 145749203}, {"filename": "/GameData/textures/lq_tech/t_blok02a.png", "start": 145749203, "end": 145754062}, {"filename": "/GameData/textures/lq_tech/t_blok03.png", "start": 145754062, "end": 145756504}, {"filename": "/GameData/textures/lq_tech/t_blok03a.png", "start": 145756504, "end": 145758359}, {"filename": "/GameData/textures/lq_tech/t_blok04.png", "start": 145758359, "end": 145761425}, {"filename": "/GameData/textures/lq_tech/t_blok04h.png", "start": 145761425, "end": 145764149}, {"filename": "/GameData/textures/lq_tech/t_blok05.png", "start": 145764149, "end": 145768437}, {"filename": "/GameData/textures/lq_tech/t_blok06.png", "start": 145768437, "end": 145771287}, {"filename": "/GameData/textures/lq_tech/t_blok06h.png", "start": 145771287, "end": 145773264}, {"filename": "/GameData/textures/lq_tech/t_blok07.png", "start": 145773264, "end": 145776050}, {"filename": "/GameData/textures/lq_tech/t_blok07a.png", "start": 145776050, "end": 145778836}, {"filename": "/GameData/textures/lq_tech/t_blok08.png", "start": 145778836, "end": 145783343}, {"filename": "/GameData/textures/lq_tech/t_blok09.png", "start": 145783343, "end": 145786663}, {"filename": "/GameData/textures/lq_tech/t_blok10.png", "start": 145786663, "end": 145792674}, {"filename": "/GameData/textures/lq_tech/t_blok10b.png", "start": 145792674, "end": 145798382}, {"filename": "/GameData/textures/lq_tech/t_blok10c.png", "start": 145798382, "end": 145801600}, {"filename": "/GameData/textures/lq_tech/t_blok11.png", "start": 145801600, "end": 145807541}, {"filename": "/GameData/textures/lq_tech/t_blok11b.png", "start": 145807541, "end": 145813146}, {"filename": "/GameData/textures/lq_tech/t_blok12c.png", "start": 145813146, "end": 145816353}, {"filename": "/GameData/textures/lq_tech/t_flat01.png", "start": 145816353, "end": 145818787}, {"filename": "/GameData/textures/lq_tech/t_flat02.png", "start": 145818787, "end": 145821152}, {"filename": "/GameData/textures/lq_tech/t_flat05.png", "start": 145821152, "end": 145829348}, {"filename": "/GameData/textures/lq_tech/t_flor1a.png", "start": 145829348, "end": 145832960}, {"filename": "/GameData/textures/lq_tech/t_flor1b.png", "start": 145832960, "end": 145836578}, {"filename": "/GameData/textures/lq_tech/t_flor2a.png", "start": 145836578, "end": 145839172}, {"filename": "/GameData/textures/lq_tech/t_flor2b.png", "start": 145839172, "end": 145841108}, {"filename": "/GameData/textures/lq_tech/t_flor2c.png", "start": 145841108, "end": 145842588}, {"filename": "/GameData/textures/lq_tech/t_flor2d.png", "start": 145842588, "end": 145843092}, {"filename": "/GameData/textures/lq_tech/t_lit01_fbr.png", "start": 145843092, "end": 145843272}, {"filename": "/GameData/textures/lq_tech/t_lit02_fbr.png", "start": 145843272, "end": 145843414}, {"filename": "/GameData/textures/lq_tech/t_lit03_fbr.png", "start": 145843414, "end": 145843549}, {"filename": "/GameData/textures/lq_tech/t_lit04_fbr.png", "start": 145843549, "end": 145843681}, {"filename": "/GameData/textures/lq_tech/t_lit05_fbr.png", "start": 145843681, "end": 145843847}, {"filename": "/GameData/textures/lq_tech/t_lit06_fbr.png", "start": 145843847, "end": 145844003}, {"filename": "/GameData/textures/lq_tech/t_lit07_fbr.png", "start": 145844003, "end": 145846306}, {"filename": "/GameData/textures/lq_tech/t_lit08_fbr.png", "start": 145846306, "end": 145846523}, {"filename": "/GameData/textures/lq_tech/t_metalsheeta.png", "start": 145846523, "end": 145853122}, {"filename": "/GameData/textures/lq_tech/t_metalsheetb.png", "start": 145853122, "end": 145864635}, {"filename": "/GameData/textures/lq_tech/t_num_0_fbr.png", "start": 145864635, "end": 145865409}, {"filename": "/GameData/textures/lq_tech/t_num_1_fbr.png", "start": 145865409, "end": 145866171}, {"filename": "/GameData/textures/lq_tech/t_num_2_fbr.png", "start": 145866171, "end": 145866966}, {"filename": "/GameData/textures/lq_tech/t_num_3_fbr.png", "start": 145866966, "end": 145867724}, {"filename": "/GameData/textures/lq_tech/t_num_4_fbr.png", "start": 145867724, "end": 145868529}, {"filename": "/GameData/textures/lq_tech/t_num_5_fbr.png", "start": 145868529, "end": 145869327}, {"filename": "/GameData/textures/lq_tech/t_num_6_fbr.png", "start": 145869327, "end": 145870124}, {"filename": "/GameData/textures/lq_tech/t_num_7_fbr.png", "start": 145870124, "end": 145870898}, {"filename": "/GameData/textures/lq_tech/t_num_8_fbr.png", "start": 145870898, "end": 145871678}, {"filename": "/GameData/textures/lq_tech/t_num_9_fbr.png", "start": 145871678, "end": 145872471}, {"filename": "/GameData/textures/lq_tech/t_num_x.png", "start": 145872471, "end": 145873279}, {"filename": "/GameData/textures/lq_tech/t_rivs01.png", "start": 145873279, "end": 145875707}, {"filename": "/GameData/textures/lq_tech/t_rivs01a.png", "start": 145875707, "end": 145878161}, {"filename": "/GameData/textures/lq_tech/t_sign1.png", "start": 145878161, "end": 145881111}, {"filename": "/GameData/textures/lq_tech/t_tech01.png", "start": 145881111, "end": 145885162}, {"filename": "/GameData/textures/lq_tech/t_tech02.png", "start": 145885162, "end": 145888265}, {"filename": "/GameData/textures/lq_tech/t_tech03.png", "start": 145888265, "end": 145904256}, {"filename": "/GameData/textures/lq_tech/t_tech04.png", "start": 145904256, "end": 145907111}, {"filename": "/GameData/textures/lq_tech/t_tech05.png", "start": 145907111, "end": 145909890}, {"filename": "/GameData/textures/lq_tech/t_tech06.png", "start": 145909890, "end": 145912702}, {"filename": "/GameData/textures/lq_tech/t_trim1a.png", "start": 145912702, "end": 145915190}, {"filename": "/GameData/textures/lq_tech/t_trim1aa.png", "start": 145915190, "end": 145917825}, {"filename": "/GameData/textures/lq_tech/t_trim1b.png", "start": 145917825, "end": 145920556}, {"filename": "/GameData/textures/lq_tech/t_trim1ba.png", "start": 145920556, "end": 145923180}, {"filename": "/GameData/textures/lq_tech/t_trim1c.png", "start": 145923180, "end": 145925965}, {"filename": "/GameData/textures/lq_tech/t_trim1ca.png", "start": 145925965, "end": 145928791}, {"filename": "/GameData/textures/lq_tech/t_trim1d.png", "start": 145928791, "end": 145931091}, {"filename": "/GameData/textures/lq_tech/t_trim1e.png", "start": 145931091, "end": 145933236}, {"filename": "/GameData/textures/lq_tech/t_trim2a.png", "start": 145933236, "end": 145936309}, {"filename": "/GameData/textures/lq_tech/t_trim2aa.png", "start": 145936309, "end": 145938961}, {"filename": "/GameData/textures/lq_tech/t_trim2b.png", "start": 145938961, "end": 145941589}, {"filename": "/GameData/textures/lq_tech/t_trim2ba.png", "start": 145941589, "end": 145944277}, {"filename": "/GameData/textures/lq_tech/t_trim2c.png", "start": 145944277, "end": 145947068}, {"filename": "/GameData/textures/lq_tech/t_trim2ca.png", "start": 145947068, "end": 145949947}, {"filename": "/GameData/textures/lq_tech/t_trim2d.png", "start": 145949947, "end": 145952409}, {"filename": "/GameData/textures/lq_tech/t_trim2e.png", "start": 145952409, "end": 145954916}, {"filename": "/GameData/textures/lq_tech/t_tris02.png", "start": 145954916, "end": 145957830}, {"filename": "/GameData/textures/lq_tech/t_wall05.png", "start": 145957830, "end": 145961156}, {"filename": "/GameData/textures/lq_tech/t_wall1a.png", "start": 145961156, "end": 145971683}, {"filename": "/GameData/textures/lq_tech/t_wall1aa.png", "start": 145971683, "end": 145982004}, {"filename": "/GameData/textures/lq_tech/t_wall1b.png", "start": 145982004, "end": 145993104}, {"filename": "/GameData/textures/lq_tech/t_wall1ba.png", "start": 145993104, "end": 146003943}, {"filename": "/GameData/textures/lq_tech/t_wall2a.png", "start": 146003943, "end": 146014727}, {"filename": "/GameData/textures/lq_tech/t_wall2aa.png", "start": 146014727, "end": 146025747}, {"filename": "/GameData/textures/lq_tech/t_wall2ab.png", "start": 146025747, "end": 146037504}, {"filename": "/GameData/textures/lq_tech/t_wall2b.png", "start": 146037504, "end": 146051742}, {"filename": "/GameData/textures/lq_tech/t_wall2ba.png", "start": 146051742, "end": 146065193}, {"filename": "/GameData/textures/lq_tech/t_wall3a.png", "start": 146065193, "end": 146073160}, {"filename": "/GameData/textures/lq_tech/t_wall3aa.png", "start": 146073160, "end": 146081427}, {"filename": "/GameData/textures/lq_tech/t_wall3b.png", "start": 146081427, "end": 146089749}, {"filename": "/GameData/textures/lq_tech/t_wall3ba.png", "start": 146089749, "end": 146098712}, {"filename": "/GameData/textures/lq_tech/t_wall6a.png", "start": 146098712, "end": 146101946}, {"filename": "/GameData/textures/lq_tech/t_wall6b.png", "start": 146101946, "end": 146104960}, {"filename": "/GameData/textures/lq_tech/t_wall6c.png", "start": 146104960, "end": 146108636}, {"filename": "/GameData/textures/lq_tech/t_wall6d.png", "start": 146108636, "end": 146112339}, {"filename": "/GameData/textures/lq_tech/t_wall6e.png", "start": 146112339, "end": 146116135}, {"filename": "/GameData/textures/lq_tech/t_wall7a.png", "start": 146116135, "end": 146127027}, {"filename": "/GameData/textures/lq_tech/t_wall7b.png", "start": 146127027, "end": 146134468}, {"filename": "/GameData/textures/lq_tech/t_wire01.png", "start": 146134468, "end": 146137404}, {"filename": "/GameData/textures/lq_tech/t_wire02.png", "start": 146137404, "end": 146140939}, {"filename": "/GameData/textures/lq_tech/t_wire03.png", "start": 146140939, "end": 146144539}, {"filename": "/GameData/textures/lq_tech/tech04_1.png", "start": 146144539, "end": 146145362}, {"filename": "/GameData/textures/lq_tech/tech04_3.png", "start": 146145362, "end": 146146831}, {"filename": "/GameData/textures/lq_tech/tech08_1.png", "start": 146146831, "end": 146157884}, {"filename": "/GameData/textures/lq_tech/tech08_2.png", "start": 146157884, "end": 146168937}, {"filename": "/GameData/textures/lq_tech/tech10_3.png", "start": 146168937, "end": 146172793}, {"filename": "/GameData/textures/lq_tech/tech14-1.png", "start": 146172793, "end": 146183064}, {"filename": "/GameData/textures/lq_tech/techbasetextures.txt", "start": 146183064, "end": 146183621}, {"filename": "/GameData/textures/lq_tech/techeye1_fbr.png", "start": 146183621, "end": 146186965}, {"filename": "/GameData/textures/lq_tech/techeye2_fbr.png", "start": 146186965, "end": 146190367}, {"filename": "/GameData/textures/lq_tech/tek_door1.png", "start": 146190367, "end": 146202513}, {"filename": "/GameData/textures/lq_tech/tek_door2.png", "start": 146202513, "end": 146214564}, {"filename": "/GameData/textures/lq_tech/tek_flr3.png", "start": 146214564, "end": 146217945}, {"filename": "/GameData/textures/lq_tech/tek_grate.png", "start": 146217945, "end": 146220823}, {"filename": "/GameData/textures/lq_tech/tek_lit1_fbr.png", "start": 146220823, "end": 146222458}, {"filename": "/GameData/textures/lq_tech/tek_lit2_fbr.png", "start": 146222458, "end": 146223444}, {"filename": "/GameData/textures/lq_tech/tek_lit3_fbr.png", "start": 146223444, "end": 146225286}, {"filename": "/GameData/textures/lq_tech/tek_lit4_fbr.png", "start": 146225286, "end": 146226413}, {"filename": "/GameData/textures/lq_tech/tek_pip1_fbr.png", "start": 146226413, "end": 146229495}, {"filename": "/GameData/textures/lq_tech/tek_pipe1.png", "start": 146229495, "end": 146232295}, {"filename": "/GameData/textures/lq_tech/tek_pipe2.png", "start": 146232295, "end": 146233954}, {"filename": "/GameData/textures/lq_tech/tek_trm1.png", "start": 146233954, "end": 146236412}, {"filename": "/GameData/textures/lq_tech/tek_trm3.png", "start": 146236412, "end": 146239022}, {"filename": "/GameData/textures/lq_tech/tek_wall4_1.png", "start": 146239022, "end": 146253467}, {"filename": "/GameData/textures/lq_tech/tele_frame1.png", "start": 146253467, "end": 146259622}, {"filename": "/GameData/textures/lq_tech/tele_frame2.png", "start": 146259622, "end": 146261573}, {"filename": "/GameData/textures/lq_tech/tele_frame3.png", "start": 146261573, "end": 146265560}, {"filename": "/GameData/textures/lq_tech/telepad1_fbr.png", "start": 146265560, "end": 146267536}, {"filename": "/GameData/textures/lq_tech/tlight11_fbr.png", "start": 146267536, "end": 146269162}, {"filename": "/GameData/textures/lq_tech/tlight12_fbr.png", "start": 146269162, "end": 146270857}, {"filename": "/GameData/textures/lq_tech/tlight13_fbr.png", "start": 146270857, "end": 146272916}, {"filename": "/GameData/textures/lq_tech/tlightblfb_fbr.png", "start": 146272916, "end": 146273489}, {"filename": "/GameData/textures/lq_tech/tlightfb_fbr.png", "start": 146273489, "end": 146274079}, {"filename": "/GameData/textures/lq_tech/tlightnb.png", "start": 146274079, "end": 146274677}, {"filename": "/GameData/textures/lq_tech/tlightrdfb_fbr.png", "start": 146274677, "end": 146275279}, {"filename": "/GameData/textures/lq_tech/treadplatemetal.png", "start": 146275279, "end": 146290033}, {"filename": "/GameData/textures/lq_tech/twall2_3.png", "start": 146290033, "end": 146293541}, {"filename": "/GameData/textures/lq_tech/w17_1.png", "start": 146293541, "end": 146310565}, {"filename": "/GameData/textures/lq_tech/w94_1.png", "start": 146310565, "end": 146323513}, {"filename": "/GameData/textures/lq_tech/z_exit_fbr.png", "start": 146323513, "end": 146325074}, {"filename": "/GameData/textures/lq_terra/afloor1_3.png", "start": 146325074, "end": 146328072}, {"filename": "/GameData/textures/lq_terra/asphalt.png", "start": 146328072, "end": 146345532}, {"filename": "/GameData/textures/lq_terra/azfloor1_1.png", "start": 146345532, "end": 146348551}, {"filename": "/GameData/textures/lq_terra/badlawn.png", "start": 146348551, "end": 146390360}, {"filename": "/GameData/textures/lq_terra/cracks1-1.png", "start": 146390360, "end": 146393379}, {"filename": "/GameData/textures/lq_terra/darkrock.png", "start": 146393379, "end": 146415948}, {"filename": "/GameData/textures/lq_terra/grass1.png", "start": 146415948, "end": 146426519}, {"filename": "/GameData/textures/lq_terra/gravel1.png", "start": 146426519, "end": 146439873}, {"filename": "/GameData/textures/lq_terra/gravel2.png", "start": 146439873, "end": 146454770}, {"filename": "/GameData/textures/lq_terra/grk_leaf1_1.png", "start": 146454770, "end": 146458763}, {"filename": "/GameData/textures/lq_terra/grk_leaf1_2.png", "start": 146458763, "end": 146462500}, {"filename": "/GameData/textures/lq_terra/marbbrn128.png", "start": 146462500, "end": 146472742}, {"filename": "/GameData/textures/lq_terra/may_drt1_1.png", "start": 146472742, "end": 146475638}, {"filename": "/GameData/textures/lq_terra/may_drt1_2.png", "start": 146475638, "end": 146478492}, {"filename": "/GameData/textures/lq_terra/may_drt2_2.png", "start": 146478492, "end": 146481236}, {"filename": "/GameData/textures/lq_terra/may_rck1_1.png", "start": 146481236, "end": 146495336}, {"filename": "/GameData/textures/lq_terra/may_rck1_2.png", "start": 146495336, "end": 146506014}, {"filename": "/GameData/textures/lq_terra/may_rck1_3.png", "start": 146506014, "end": 146518857}, {"filename": "/GameData/textures/lq_terra/may_slat1_1.png", "start": 146518857, "end": 146521705}, {"filename": "/GameData/textures/lq_terra/med_bigdirt.png", "start": 146521705, "end": 146676288}, {"filename": "/GameData/textures/lq_terra/med_bigdirt2.png", "start": 146676288, "end": 146830439}, {"filename": "/GameData/textures/lq_terra/med_bigdirt3.png", "start": 146830439, "end": 146984390}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_1.png", "start": 146984390, "end": 146998771}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_1a.png", "start": 146998771, "end": 147015745}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_2.png", "start": 147015745, "end": 147030184}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_2a.png", "start": 147030184, "end": 147046999}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_1.png", "start": 147046999, "end": 147060968}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_1a.png", "start": 147060968, "end": 147078279}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_2.png", "start": 147078279, "end": 147093353}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_2a.png", "start": 147093353, "end": 147110457}, {"filename": "/GameData/textures/lq_terra/med_cracks1.png", "start": 147110457, "end": 147123919}, {"filename": "/GameData/textures/lq_terra/med_flat1.png", "start": 147123919, "end": 147136828}, {"filename": "/GameData/textures/lq_terra/med_flat12.png", "start": 147136828, "end": 147148216}, {"filename": "/GameData/textures/lq_terra/med_flat15.png", "start": 147148216, "end": 147161317}, {"filename": "/GameData/textures/lq_terra/med_flat16.png", "start": 147161317, "end": 147174159}, {"filename": "/GameData/textures/lq_terra/med_flat2.png", "start": 147174159, "end": 147185436}, {"filename": "/GameData/textures/lq_terra/med_flat3.png", "start": 147185436, "end": 147198801}, {"filename": "/GameData/textures/lq_terra/med_flat4.png", "start": 147198801, "end": 147208750}, {"filename": "/GameData/textures/lq_terra/med_flat5.png", "start": 147208750, "end": 147219835}, {"filename": "/GameData/textures/lq_terra/med_flat5a.png", "start": 147219835, "end": 147230579}, {"filename": "/GameData/textures/lq_terra/med_flat6.png", "start": 147230579, "end": 147239564}, {"filename": "/GameData/textures/lq_terra/med_flat7.png", "start": 147239564, "end": 147251022}, {"filename": "/GameData/textures/lq_terra/med_plaster2.png", "start": 147251022, "end": 147260674}, {"filename": "/GameData/textures/lq_terra/med_rock1.png", "start": 147260674, "end": 147311931}, {"filename": "/GameData/textures/lq_terra/med_rock10.png", "start": 147311931, "end": 147321226}, {"filename": "/GameData/textures/lq_terra/med_rock10a.png", "start": 147321226, "end": 147330540}, {"filename": "/GameData/textures/lq_terra/med_rock10b.png", "start": 147330540, "end": 147343905}, {"filename": "/GameData/textures/lq_terra/med_rock10c.png", "start": 147343905, "end": 147356240}, {"filename": "/GameData/textures/lq_terra/med_rock2.png", "start": 147356240, "end": 147398117}, {"filename": "/GameData/textures/lq_terra/med_rock3.png", "start": 147398117, "end": 147444061}, {"filename": "/GameData/textures/lq_terra/med_rock3_bump.png", "start": 147444061, "end": 147560155}, {"filename": "/GameData/textures/lq_terra/med_rock4.png", "start": 147560155, "end": 147571511}, {"filename": "/GameData/textures/lq_terra/med_rock5.png", "start": 147571511, "end": 147584043}, {"filename": "/GameData/textures/lq_terra/med_rock9.png", "start": 147584043, "end": 147595067}, {"filename": "/GameData/textures/lq_terra/ret_plaster1.png", "start": 147595067, "end": 147606344}, {"filename": "/GameData/textures/lq_terra/rock1_1.png", "start": 147606344, "end": 147656715}, {"filename": "/GameData/textures/lq_terra/rock1_1b.png", "start": 147656715, "end": 147667730}, {"filename": "/GameData/textures/lq_terra/rock1_2.png", "start": 147667730, "end": 147709773}, {"filename": "/GameData/textures/lq_terra/rocks07.png", "start": 147709773, "end": 147720797}, {"filename": "/GameData/textures/lq_terra/rocks11d.png", "start": 147720797, "end": 147730111}, {"filename": "/GameData/textures/lq_terra/rocks11e.png", "start": 147730111, "end": 147739406}, {"filename": "/GameData/textures/lq_terra/sand.png", "start": 147739406, "end": 147765493}, {"filename": "/GameData/textures/lq_terra/snow1.png", "start": 147765493, "end": 147767356}, {"filename": "/GameData/textures/lq_terra/uwall1_2.png", "start": 147767356, "end": 147795007}, {"filename": "/GameData/textures/lq_terra/vines1.png", "start": 147795007, "end": 147800062}, {"filename": "/GameData/textures/lq_utility/black.png", "start": 147800062, "end": 147800610}, {"filename": "/GameData/textures/lq_utility/clip.png", "start": 147800610, "end": 147801116}, {"filename": "/GameData/textures/lq_utility/hint.png", "start": 147801116, "end": 147801982}, {"filename": "/GameData/textures/lq_utility/hintskip.png", "start": 147801982, "end": 147802875}, {"filename": "/GameData/textures/lq_utility/light_fbr.png", "start": 147802875, "end": 147803791}, {"filename": "/GameData/textures/lq_utility/origin.png", "start": 147803791, "end": 147804274}, {"filename": "/GameData/textures/lq_utility/skip.png", "start": 147804274, "end": 147804762}, {"filename": "/GameData/textures/lq_utility/star_lavaskip.png", "start": 147804762, "end": 147805875}, {"filename": "/GameData/textures/lq_utility/star_slimeskip.png", "start": 147805875, "end": 147806948}, {"filename": "/GameData/textures/lq_utility/star_waterskip.png", "start": 147806948, "end": 147808688}, {"filename": "/GameData/textures/lq_utility/trigger.png", "start": 147808688, "end": 147809185}, {"filename": "/GameData/textures/lq_wood/crate4.png", "start": 147809185, "end": 147812327}, {"filename": "/GameData/textures/lq_wood/crwdh6.png", "start": 147812327, "end": 147817335}, {"filename": "/GameData/textures/lq_wood/crwdl12.png", "start": 147817335, "end": 147819768}, {"filename": "/GameData/textures/lq_wood/crwds6.png", "start": 147819768, "end": 147820794}, {"filename": "/GameData/textures/lq_wood/may_crate3-small.png", "start": 147820794, "end": 147821785}, {"filename": "/GameData/textures/lq_wood/may_crate3.png", "start": 147821785, "end": 147824848}, {"filename": "/GameData/textures/lq_wood/may_wood1_1.png", "start": 147824848, "end": 147827688}, {"filename": "/GameData/textures/lq_wood/may_wood1_2.png", "start": 147827688, "end": 147830537}, {"filename": "/GameData/textures/lq_wood/med_ret_wood1.png", "start": 147830537, "end": 147837506}, {"filename": "/GameData/textures/lq_wood/med_wood1.png", "start": 147837506, "end": 147864874}, {"filename": "/GameData/textures/lq_wood/med_wood2.png", "start": 147864874, "end": 147872387}, {"filename": "/GameData/textures/lq_wood/med_wood2_plk1.png", "start": 147872387, "end": 147884085}, {"filename": "/GameData/textures/lq_wood/med_wood2_plk2.png", "start": 147884085, "end": 147896577}, {"filename": "/GameData/textures/lq_wood/med_wood3.png", "start": 147896577, "end": 147902577}, {"filename": "/GameData/textures/lq_wood/med_wood3_plk1.png", "start": 147902577, "end": 147915792}, {"filename": "/GameData/textures/lq_wood/med_wood4.png", "start": 147915792, "end": 147922916}, {"filename": "/GameData/textures/lq_wood/med_wood5.png", "start": 147922916, "end": 147930714}, {"filename": "/GameData/textures/lq_wood/med_wood6.png", "start": 147930714, "end": 147937494}, {"filename": "/GameData/textures/lq_wood/med_wood7.png", "start": 147937494, "end": 147946533}, {"filename": "/GameData/textures/lq_wood/med_wood8.png", "start": 147946533, "end": 147954497}, {"filename": "/GameData/textures/lq_wood/med_wood_riv1.png", "start": 147954497, "end": 147961122}, {"filename": "/GameData/textures/lq_wood/med_wood_riv1b.png", "start": 147961122, "end": 147967201}, {"filename": "/GameData/textures/lq_wood/med_wood_riv1c.png", "start": 147967201, "end": 147972325}, {"filename": "/GameData/textures/lq_wood/med_wood_riv2.png", "start": 147972325, "end": 147978735}, {"filename": "/GameData/textures/lq_wood/med_wood_riv2b.png", "start": 147978735, "end": 147985155}, {"filename": "/GameData/textures/lq_wood/med_wood_riv2c.png", "start": 147985155, "end": 147990527}, {"filename": "/GameData/textures/lq_wood/plank1.png", "start": 147990527, "end": 147996941}, {"filename": "/GameData/textures/lq_wood/plank1s.png", "start": 147996941, "end": 147998822}, {"filename": "/GameData/textures/lq_wood/plank2.png", "start": 147998822, "end": 148005402}, {"filename": "/GameData/textures/lq_wood/plank2s.png", "start": 148005402, "end": 148007291}, {"filename": "/GameData/textures/lq_wood/plank3.png", "start": 148007291, "end": 148014151}, {"filename": "/GameData/textures/lq_wood/plank3s.png", "start": 148014151, "end": 148016132}, {"filename": "/GameData/textures/lq_wood/plank4.png", "start": 148016132, "end": 148022578}, {"filename": "/GameData/textures/lq_wood/plank4s.png", "start": 148022578, "end": 148024468}, {"filename": "/GameData/textures/lq_wood/plank5.png", "start": 148024468, "end": 148030117}, {"filename": "/GameData/textures/lq_wood/sq_wood_1.png", "start": 148030117, "end": 148046158}, {"filename": "/GameData/textures/lq_wood/sq_wood_2.png", "start": 148046158, "end": 148049046}, {"filename": "/GameData/textures/lq_wood/sq_wood_2a.png", "start": 148049046, "end": 148052415}, {"filename": "/GameData/textures/lq_wood/wood_1.png", "start": 148052415, "end": 148056593}, {"filename": "/GameData/textures/lq_wood/wood_2.png", "start": 148056593, "end": 148060493}, {"filename": "/GameData/textures/lq_wood/woodbark128.png", "start": 148060493, "end": 148070700}, {"filename": "/GameData/textures/lq_wood/woodbark1m28.png", "start": 148070700, "end": 148080149}, {"filename": "/GameData/textures/lq_wood/woodbark64.png", "start": 148080149, "end": 148083086}, {"filename": "/GameData/textures/lq_wood/woodbarkA128.png", "start": 148083086, "end": 148091784}, {"filename": "/GameData/textures/lq_wood/woodbarkm64.png", "start": 148091784, "end": 148094461}, {"filename": "/GameData/textures/lq_wood/woodend.png", "start": 148094461, "end": 148096964}, {"filename": "/GameData/textures/lq_wood/woodring128.png", "start": 148096964, "end": 148105388}, {"filename": "/GameData/textures/lq_wood/woodring64.png", "start": 148105388, "end": 148107849}, {"filename": "/GameData/textures/lq_wood/woodringm128.png", "start": 148107849, "end": 148118148}, {"filename": "/GameData/textures/lq_wood/woodringm64.png", "start": 148118148, "end": 148121407}, {"filename": "/GameData/textures/metal/metal1.png", "start": 148121407, "end": 148545463}, {"filename": "/GameData/textures/muzzle_t.png", "start": 148545463, "end": 148554871}, {"filename": "/GameData/textures/muzzle_t_em.png", "start": 148554871, "end": 148564279}, {"filename": "/GameData/textures/noise/grainy5_256.png", "start": 148564279, "end": 148790321}, {"filename": "/GameData/textures/particles/blood.png", "start": 148790321, "end": 148794034}, {"filename": "/GameData/textures/particles/smoke.png", "start": 148794034, "end": 148797904}, {"filename": "/GameData/textures/particles/trail.png", "start": 148797904, "end": 148816774}, {"filename": "/GameData/textures/particles/wood.png", "start": 148816774, "end": 148832257}, {"filename": "/GameData/textures/pp/lut.png", "start": 148832257, "end": 148833880}, {"filename": "/GameData/textures/pp/main.png", "start": 148833880, "end": 148871932}, {"filename": "/GameData/textures/pp/pal.png_out.png", "start": 148871932, "end": 148878282}, {"filename": "/GameData/textures/pp/sin.png", "start": 148878282, "end": 148879607}, {"filename": "/GameData/textures/shirt.png", "start": 148879607, "end": 149071358}, {"filename": "/GameData/textures/skies/skybox1_cube.png", "start": 149071358, "end": 150284325}, {"filename": "/GameData/textures/skies/skybox2_overcast_cube.png", "start": 150284325, "end": 150932164}, {"filename": "/GameData/textures/tormentPack/+0str_bloodfall.png", "start": 150932164, "end": 150934241}, {"filename": "/GameData/textures/tormentPack/+1str_bloodfall.png", "start": 150934241, "end": 150936275}, {"filename": "/GameData/textures/tormentPack/+2str_bloodfall.png", "start": 150936275, "end": 150938373}, {"filename": "/GameData/textures/tormentPack/+3str_bloodfall.png", "start": 150938373, "end": 150940444}, {"filename": "/GameData/textures/tormentPack/+4str_bloodfall.png", "start": 150940444, "end": 150942504}, {"filename": "/GameData/textures/tormentPack/+5str_bloodfall.png", "start": 150942504, "end": 150944525}, {"filename": "/GameData/textures/tormentPack/+6str_bloodfall.png", "start": 150944525, "end": 150946571}, {"filename": "/GameData/textures/tormentPack/+7str_bloodfall.png", "start": 150946571, "end": 150948640}, {"filename": "/GameData/textures/tormentPack/str_blood.png", "start": 150948640, "end": 150950648}, {"filename": "/GameData/textures/tormentPack/str_blood_large.png", "start": 150950648, "end": 150973198}, {"filename": "/GameData/textures/tormentPack/str_bloodvein1.png", "start": 150973198, "end": 150999873}, {"filename": "/GameData/textures/tormentPack/str_bloodvein2.png", "start": 150999873, "end": 151031906}, {"filename": "/GameData/textures/tormentPack/str_bloodvein3.png", "start": 151031906, "end": 151065577}, {"filename": "/GameData/textures/tormentPack/str_bloodvein4.png", "start": 151065577, "end": 151097219}, {"filename": "/GameData/textures/tormentPack/str_bloodvein5.png", "start": 151097219, "end": 151130710}, {"filename": "/GameData/textures/tormentPack/str_bloodvein6.png", "start": 151130710, "end": 151165184}, {"filename": "/GameData/textures/tormentPack/str_bloodvein7.png", "start": 151165184, "end": 151200075}, {"filename": "/GameData/textures/tormentPack/str_bloodvein8.png", "start": 151200075, "end": 151228224}, {"filename": "/GameData/textures/tormentPack/str_bloodvein9.png", "start": 151228224, "end": 151263677}, {"filename": "/GameData/textures/tormentPack/str_metalflr1.png", "start": 151263677, "end": 151300371}, {"filename": "/GameData/textures/tormentPack/str_metalflr2.png", "start": 151300371, "end": 151336946}, {"filename": "/GameData/textures/tormentPack/str_metalflr3.png", "start": 151336946, "end": 151367939}, {"filename": "/GameData/textures/tormentPack/str_metalflr4.png", "start": 151367939, "end": 151398109}, {"filename": "/GameData/textures/tormentPack/str_metalflr5.png", "start": 151398109, "end": 151437559}, {"filename": "/GameData/textures/tormentPack/str_metalflr6.png", "start": 151437559, "end": 151476719}, {"filename": "/GameData/textures/tormentPack/str_metalflr7.png", "start": 151476719, "end": 151518411}, {"filename": "/GameData/textures/tormentPack/str_metalflr8.png", "start": 151518411, "end": 151558829}, {"filename": "/GameData/textures/tormentPack/str_metalgen1.png", "start": 151558829, "end": 151588099}, {"filename": "/GameData/textures/tormentPack/str_metalgen2.png", "start": 151588099, "end": 151617615}, {"filename": "/GameData/textures/tormentPack/str_metalgen3.png", "start": 151617615, "end": 151647091}, {"filename": "/GameData/textures/tormentPack/str_metalgen4.png", "start": 151647091, "end": 151671914}, {"filename": "/GameData/textures/tormentPack/str_metalgen5.png", "start": 151671914, "end": 151696784}, {"filename": "/GameData/textures/tormentPack/str_metalgen6.png", "start": 151696784, "end": 151720874}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl1.png", "start": 151720874, "end": 151750259}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl2.png", "start": 151750259, "end": 151782989}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl3.png", "start": 151782989, "end": 151809607}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl4.png", "start": 151809607, "end": 151838871}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl5.png", "start": 151838871, "end": 151873246}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl6.png", "start": 151873246, "end": 151911741}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl7.png", "start": 151911741, "end": 151947891}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl8.png", "start": 151947891, "end": 151985884}, {"filename": "/GameData/textures/tormentPack/str_metalpan1.png", "start": 151985884, "end": 152020744}, {"filename": "/GameData/textures/tormentPack/str_metalpan2.png", "start": 152020744, "end": 152056285}, {"filename": "/GameData/textures/tormentPack/str_metalpan3.png", "start": 152056285, "end": 152086721}, {"filename": "/GameData/textures/tormentPack/str_metalpan4.png", "start": 152086721, "end": 152117681}, {"filename": "/GameData/textures/tormentPack/str_metalpan5.png", "start": 152117681, "end": 152154053}, {"filename": "/GameData/textures/tormentPack/str_metalpan6.png", "start": 152154053, "end": 152192095}, {"filename": "/GameData/textures/tormentPack/str_metalpan7.png", "start": 152192095, "end": 152231324}, {"filename": "/GameData/textures/tormentPack/str_metalpan8.png", "start": 152231324, "end": 152270418}, {"filename": "/GameData/textures/tormentPack/str_rotwoodgen1.png", "start": 152270418, "end": 152288997}, {"filename": "/GameData/textures/tormentPack/str_rotwoodgen2.png", "start": 152288997, "end": 152308293}, {"filename": "/GameData/textures/tormentPack/str_rotwoodgen3.png", "start": 152308293, "end": 152328347}, {"filename": "/GameData/textures/tormentPack/str_stonebrk1.png", "start": 152328347, "end": 152353739}, {"filename": "/GameData/textures/tormentPack/str_stonebrk2.png", "start": 152353739, "end": 152380741}, {"filename": "/GameData/textures/tormentPack/str_stonebrk3.png", "start": 152380741, "end": 152403371}, {"filename": "/GameData/textures/tormentPack/str_stonebrk4.png", "start": 152403371, "end": 152427424}, {"filename": "/GameData/textures/tormentPack/str_stonebrk5.png", "start": 152427424, "end": 152458508}, {"filename": "/GameData/textures/tormentPack/str_stonebrk6.png", "start": 152458508, "end": 152491180}, {"filename": "/GameData/textures/tormentPack/str_stonebrk7.png", "start": 152491180, "end": 152519704}, {"filename": "/GameData/textures/tormentPack/str_stonebrk8.png", "start": 152519704, "end": 152549956}, {"filename": "/GameData/textures/tormentPack/str_stoneflr1.png", "start": 152549956, "end": 152574112}, {"filename": "/GameData/textures/tormentPack/str_stoneflr2.png", "start": 152574112, "end": 152602018}, {"filename": "/GameData/textures/tormentPack/str_stoneflr3.png", "start": 152602018, "end": 152623023}, {"filename": "/GameData/textures/tormentPack/str_stoneflr4.png", "start": 152623023, "end": 152644167}, {"filename": "/GameData/textures/tormentPack/str_stoneflr5.png", "start": 152644167, "end": 152675699}, {"filename": "/GameData/textures/tormentPack/str_stoneflr6.png", "start": 152675699, "end": 152709982}, {"filename": "/GameData/textures/tormentPack/str_stonegen1.png", "start": 152709982, "end": 152729005}, {"filename": "/GameData/textures/tormentPack/str_stonegen2.png", "start": 152729005, "end": 152748376}, {"filename": "/GameData/textures/tormentPack/str_stonegen3.png", "start": 152748376, "end": 152770148}, {"filename": "/GameData/textures/tormentPack/str_stonegen4.png", "start": 152770148, "end": 152795193}, {"filename": "/GameData/textures/tormentPack/str_stonegen5.png", "start": 152795193, "end": 152820581}, {"filename": "/GameData/textures/tormentPack/str_stonegen6.png", "start": 152820581, "end": 152848018}, {"filename": "/GameData/textures/tormentPack/str_stonerubble.png", "start": 152848018, "end": 152876412}, {"filename": "/GameData/textures/tormentPack/str_stonewall1.png", "start": 152876412, "end": 152899669}, {"filename": "/GameData/textures/tormentPack/str_stonewall2.png", "start": 152899669, "end": 152924474}, {"filename": "/GameData/textures/tormentPack/str_stonewall3.png", "start": 152924474, "end": 152949137}, {"filename": "/GameData/textures/tormentPack/str_stonewall4.png", "start": 152949137, "end": 152975607}, {"filename": "/GameData/textures/tormentPack/str_stonewall5.png", "start": 152975607, "end": 153003074}, {"filename": "/GameData/textures/tormentPack/str_stonewall6.png", "start": 153003074, "end": 153031242}, {"filename": "/GameData/textures/tormentPack/str_stonewall7.png", "start": 153031242, "end": 153060631}, {"filename": "/GameData/textures/tormentPack/str_stonewall8.png", "start": 153060631, "end": 153091527}, {"filename": "/GameData/textures/tormentPack/{str_bloodgunk.png", "start": 153091527, "end": 153117976}, {"filename": "/GameData/textures/tormentPack/{str_bloodweb1.png", "start": 153117976, "end": 153135216}, {"filename": "/GameData/textures/tormentPack/{str_bloodweb2.png", "start": 153135216, "end": 153144167}, {"filename": "/GameData/textures/tormentPack/{str_bloodweb3.png", "start": 153144167, "end": 153150178}, {"filename": "/GameData/textures/tormentPack/{str_grating1.png", "start": 153150178, "end": 153158759}, {"filename": "/GameData/textures/tormentPack/{str_grating2.png", "start": 153158759, "end": 153173962}, {"filename": "/GameData/textures/tormentPack/{str_grating3.png", "start": 153173962, "end": 153193310}, {"filename": "/GameData/textures/tormentPack/{str_grating4.png", "start": 153193310, "end": 153205467}, {"filename": "/GameData/textures/tormentPack/{str_grating5.png", "start": 153205467, "end": 153226972}, {"filename": "/GameData/textures/tormentPack/{str_grating6.png", "start": 153226972, "end": 153255218}, {"filename": "/GameData/textures/ui/circle.png", "start": 153255218, "end": 153258673}, {"filename": "/GameData/textures/ui/crosshair.png", "start": 153258673, "end": 153262788}, {"filename": "/GameData/textures/ui/crosshair_cross.png", "start": 153262788, "end": 153266551}, {"filename": "/GameData/textures/ui/crosshair_dot.png", "start": 153266551, "end": 153268225}, {"filename": "/GameData/textures/ui/crosshair_line.png", "start": 153268225, "end": 153270164}, {"filename": "/GameData/textures/ui/debuffs/disb.png", "start": 153270164, "end": 153736166}, {"filename": "/GameData/textures/ui/debuffs/qs.png", "start": 153736166, "end": 154223486}, {"filename": "/GameData/textures/ui/debuffs/stun.png", "start": 154223486, "end": 154717363}, {"filename": "/GameData/textures/ui/test_button.png", "start": 154717363, "end": 154720759}, {"filename": "/GameData/textures/ui/white.png", "start": 154720759, "end": 154720880}, {"filename": "/GameData/textures/wall/brickWall1.png", "start": 154720880, "end": 154729096}, {"filename": "/GameData/textures/wall/brickWall2.png", "start": 154729096, "end": 154737220}, {"filename": "/GameData/textures/wall/brickWall3.png", "start": 154737220, "end": 154745103}, {"filename": "/GameData/textures/water/Water1_t.png", "start": 154745103, "end": 155058204}, {"filename": "/GameData/textures/wood/wood1.png", "start": 155058204, "end": 155406556}, {"filename": "/GameData/ui/base.rcss", "start": 155406556, "end": 155407599}, {"filename": "/GameData/ui/pause.rml", "start": 155407599, "end": 155408614}, {"filename": "/GameData/ui/settings.rml", "start": 155408614, "end": 155409482}, {"filename": "/GameData/ui/style.rcss", "start": 155409482, "end": 155414481}, {"filename": "/GameData/ui/test.rml", "start": 155414481, "end": 155415607}, {"filename": "/GameData/ui/videoSettings.rml", "start": 155415607, "end": 155418092}, {"filename": "/GameData/videos/meowl.mpg", "start": 155418092, "end": 159358444}, {"filename": "/GameData/videos/writeshit.mpg", "start": 159358444, "end": 165449196}], "remote_package_size": 165449196});

  })();

// end include: C:\Users\bogda_\AppData\Local\Temp\tmptd0r_bya.js
// include: C:\Users\bogda_\AppData\Local\Temp\tmpgn39do85.js

    // All the pre-js content up to here must remain later on, we need to run
    // it.
    if ((typeof ENVIRONMENT_IS_WASM_WORKER != 'undefined' && ENVIRONMENT_IS_WASM_WORKER) || (typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD) || (typeof ENVIRONMENT_IS_AUDIO_WORKLET != 'undefined' && ENVIRONMENT_IS_AUDIO_WORKLET)) Module['preRun'] = [];
    var necessaryPreJSTasks = Module['preRun'].slice();
  // end include: C:\Users\bogda_\AppData\Local\Temp\tmpgn39do85.js
// include: C:\Users\bogda_\AppData\Local\Temp\tmpd3_bphy8.js

    if (!Module['preRun']) throw 'Module.preRun should exist because file support used it; did a pre-js delete it?';
    necessaryPreJSTasks.forEach((task) => {
      if (Module['preRun'].indexOf(task) < 0) throw 'All preRun tasks that exist before user pre-js code should remain after; did you replace Module or modify Module.preRun?';
    });
  // end include: C:\Users\bogda_\AppData\Local\Temp\tmpd3_bphy8.js


var arguments_ = [];
var thisProgram = './this.program';
var quit_ = (status, toThrow) => {
  throw toThrow;
};

// In MODULARIZE mode _scriptName needs to be captured already at the very top of the page immediately when the page is parsed, so it is generated there
// before the page load. In non-MODULARIZE modes generate it here.
var _scriptName = typeof document != 'undefined' ? document.currentScript?.src : undefined;

if (typeof __filename != 'undefined') { // Node
  _scriptName = __filename;
} else
if (ENVIRONMENT_IS_WORKER) {
  _scriptName = self.location.href;
}

// `/` should be present at the end if `scriptDirectory` is not empty
var scriptDirectory = '';
function locateFile(path) {
  if (Module['locateFile']) {
    return Module['locateFile'](path, scriptDirectory);
  }
  return scriptDirectory + path;
}

// Hooks that are implemented differently in different runtime environments.
var readAsync, readBinary;

if (ENVIRONMENT_IS_NODE) {
  const isNode = typeof process == 'object' && process.versions?.node && process.type != 'renderer';
  if (!isNode) throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');

  var nodeVersion = process.versions.node;
  var numericVersion = nodeVersion.split('.').slice(0, 3);
  numericVersion = (numericVersion[0] * 10000) + (numericVersion[1] * 100) + (numericVersion[2].split('-')[0] * 1);
  var minVersion = 160000;
  if (numericVersion < 160000) {
    throw new Error('This emscripten-generated code requires node v16.0.0 (detected v' + nodeVersion + ')');
  }

  // These modules will usually be used on Node.js. Load them eagerly to avoid
  // the complexity of lazy-loading.
  var fs = require('fs');
  var nodePath = require('path');

  scriptDirectory = __dirname + '/';

// include: node_shell_read.js
readBinary = (filename) => {
  // We need to re-wrap `file://` strings to URLs.
  filename = isFileURI(filename) ? new URL(filename) : filename;
  var ret = fs.readFileSync(filename);
  assert(Buffer.isBuffer(ret));
  return ret;
};

readAsync = async (filename, binary = true) => {
  // See the comment in the `readBinary` function.
  filename = isFileURI(filename) ? new URL(filename) : filename;
  var ret = fs.readFileSync(filename, binary ? undefined : 'utf8');
  assert(binary ? Buffer.isBuffer(ret) : typeof ret == 'string');
  return ret;
};
// end include: node_shell_read.js
  if (process.argv.length > 1) {
    thisProgram = process.argv[1].replace(/\\/g, '/');
  }

  arguments_ = process.argv.slice(2);

  // MODULARIZE will export the module in the proper place outside, we don't need to export here
  if (typeof module != 'undefined') {
    module['exports'] = Module;
  }

  quit_ = (status, toThrow) => {
    process.exitCode = status;
    throw toThrow;
  };

} else
if (ENVIRONMENT_IS_SHELL) {

  const isNode = typeof process == 'object' && process.versions?.node && process.type != 'renderer';
  if (isNode || typeof window == 'object' || typeof WorkerGlobalScope != 'undefined') throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');

} else

// Note that this includes Node.js workers when relevant (pthreads is enabled).
// Node.js workers are detected as a combination of ENVIRONMENT_IS_WORKER and
// ENVIRONMENT_IS_NODE.
if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
  try {
    scriptDirectory = new URL('.', _scriptName).href; // includes trailing slash
  } catch {
    // Must be a `blob:` or `data:` URL (e.g. `blob:http://site.com/etc/etc`), we cannot
    // infer anything from them.
  }

  if (!(typeof window == 'object' || typeof WorkerGlobalScope != 'undefined')) throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');

  {
// include: web_or_worker_shell_read.js
if (ENVIRONMENT_IS_WORKER) {
    readBinary = (url) => {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', url, false);
      xhr.responseType = 'arraybuffer';
      xhr.send(null);
      return new Uint8Array(/** @type{!ArrayBuffer} */(xhr.response));
    };
  }

  readAsync = async (url) => {
    // Fetch has some additional restrictions over XHR, like it can't be used on a file:// url.
    // See https://github.com/github/fetch/pull/92#issuecomment-140665932
    // Cordova or Electron apps are typically loaded from a file:// url.
    // So use XHR on webview if URL is a file URL.
    if (isFileURI(url)) {
      return new Promise((resolve, reject) => {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', url, true);
        xhr.responseType = 'arraybuffer';
        xhr.onload = () => {
          if (xhr.status == 200 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
            resolve(xhr.response);
            return;
          }
          reject(xhr.status);
        };
        xhr.onerror = reject;
        xhr.send(null);
      });
    }
    var response = await fetch(url, { credentials: 'same-origin' });
    if (response.ok) {
      return response.arrayBuffer();
    }
    throw new Error(response.status + ' : ' + response.url);
  };
// end include: web_or_worker_shell_read.js
  }
} else
{
  throw new Error('environment detection error');
}

var out = console.log.bind(console);
var err = console.error.bind(console);

var IDBFS = 'IDBFS is no longer included by default; build with -lidbfs.js';
var PROXYFS = 'PROXYFS is no longer included by default; build with -lproxyfs.js';
var WORKERFS = 'WORKERFS is no longer included by default; build with -lworkerfs.js';
var FETCHFS = 'FETCHFS is no longer included by default; build with -lfetchfs.js';
var ICASEFS = 'ICASEFS is no longer included by default; build with -licasefs.js';
var JSFILEFS = 'JSFILEFS is no longer included by default; build with -ljsfilefs.js';
var OPFS = 'OPFS is no longer included by default; build with -lopfs.js';

var NODEFS = 'NODEFS is no longer included by default; build with -lnodefs.js';

// perform assertions in shell.js after we set up out() and err(), as otherwise
// if an assertion fails it cannot print the message

assert(!ENVIRONMENT_IS_SHELL, 'shell environment detected but not enabled at build time.  Add `shell` to `-sENVIRONMENT` to enable.');

// end include: shell.js

// include: preamble.js
// === Preamble library stuff ===

// Documentation for the public APIs defined in this file must be updated in:
//    site/source/docs/api_reference/preamble.js.rst
// A prebuilt local version of the documentation is available at:
//    site/build/text/docs/api_reference/preamble.js.txt
// You can also build docs locally as HTML or other formats in site/
// An online HTML version (which may be of a different version of Emscripten)
//    is up at http://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html

var wasmBinary;

if (typeof WebAssembly != 'object') {
  err('no native wasm support detected');
}

// Wasm globals

var wasmMemory;

//========================================
// Runtime essentials
//========================================

// whether we are quitting the application. no code should run after this.
// set in exit() and abort()
var ABORT = false;

// set by exit() and abort().  Passed to 'onExit' handler.
// NOTE: This is also used as the process return code code in shell environments
// but only when noExitRuntime is false.
var EXITSTATUS;

// In STRICT mode, we only define assert() when ASSERTIONS is set.  i.e. we
// don't define it at all in release modes.  This matches the behaviour of
// MINIMAL_RUNTIME.
// TODO(sbc): Make this the default even without STRICT enabled.
/** @type {function(*, string=)} */
function assert(condition, text) {
  if (!condition) {
    abort('Assertion failed' + (text ? ': ' + text : ''));
  }
}

// We used to include malloc/free by default in the past. Show a helpful error in
// builds with assertions.

// Memory management

var HEAP,
/** @type {!Int8Array} */
  HEAP8,
/** @type {!Uint8Array} */
  HEAPU8,
/** @type {!Int16Array} */
  HEAP16,
/** @type {!Uint16Array} */
  HEAPU16,
/** @type {!Int32Array} */
  HEAP32,
/** @type {!Uint32Array} */
  HEAPU32,
/** @type {!Float32Array} */
  HEAPF32,
/* BigInt64Array type is not correctly defined in closure
/** not-@type {!BigInt64Array} */
  HEAP64,
/* BigUint64Array type is not correctly defined in closure
/** not-t@type {!BigUint64Array} */
  HEAPU64,
/** @type {!Float64Array} */
  HEAPF64;

var runtimeInitialized = false;

/**
 * Indicates whether filename is delivered via file protocol (as opposed to http/https)
 * @noinline
 */
var isFileURI = (filename) => filename.startsWith('file://');

// include: runtime_shared.js
// include: runtime_stack_check.js
// Initializes the stack cookie. Called at the startup of main and at the startup of each thread in pthreads mode.
function writeStackCookie() {
  var max = _emscripten_stack_get_end();
  assert((max & 3) == 0);
  // If the stack ends at address zero we write our cookies 4 bytes into the
  // stack.  This prevents interference with SAFE_HEAP and ASAN which also
  // monitor writes to address zero.
  if (max == 0) {
    max += 4;
  }
  // The stack grow downwards towards _emscripten_stack_get_end.
  // We write cookies to the final two words in the stack and detect if they are
  // ever overwritten.
  HEAPU32[((max)>>2)] = 0x02135467;
  HEAPU32[(((max)+(4))>>2)] = 0x89BACDFE;
  // Also test the global address 0 for integrity.
  HEAPU32[((0)>>2)] = 1668509029;
}

function checkStackCookie() {
  if (ABORT) return;
  var max = _emscripten_stack_get_end();
  // See writeStackCookie().
  if (max == 0) {
    max += 4;
  }
  var cookie1 = HEAPU32[((max)>>2)];
  var cookie2 = HEAPU32[(((max)+(4))>>2)];
  if (cookie1 != 0x02135467 || cookie2 != 0x89BACDFE) {
    abort(`Stack overflow! Stack cookie has been overwritten at ${ptrToString(max)}, expected hex dwords 0x89BACDFE and 0x2135467, but received ${ptrToString(cookie2)} ${ptrToString(cookie1)}`);
  }
  // Also test the global address 0 for integrity.
  if (HEAPU32[((0)>>2)] != 0x63736d65 /* 'emsc' */) {
    abort('Runtime error: The application has corrupted its heap memory area (address zero)!');
  }
}
// end include: runtime_stack_check.js
// include: runtime_exceptions.js
// Base Emscripten EH error class
class EmscriptenEH extends Error {}

class EmscriptenSjLj extends EmscriptenEH {}

class CppException extends EmscriptenEH {
  constructor(excPtr) {
    super(excPtr);
    this.excPtr = excPtr;
    const excInfo = getExceptionMessage(excPtr);
    this.name = excInfo[0];
    this.message = excInfo[1];
  }
}
// end include: runtime_exceptions.js
// include: runtime_debug.js
var runtimeDebug = true; // Switch to false at runtime to disable logging at the right times

// Used by XXXXX_DEBUG settings to output debug messages.
function dbg(...args) {
  if (!runtimeDebug && typeof runtimeDebug != 'undefined') return;
  // TODO(sbc): Make this configurable somehow.  Its not always convenient for
  // logging to show up as warnings.
  console.warn(...args);
}

// Endianness check
(() => {
  var h16 = new Int16Array(1);
  var h8 = new Int8Array(h16.buffer);
  h16[0] = 0x6373;
  if (h8[0] !== 0x73 || h8[1] !== 0x63) throw 'Runtime error: expected the system to be little-endian! (Run with -sSUPPORT_BIG_ENDIAN to bypass)';
})();

function consumedModuleProp(prop) {
  if (!Object.getOwnPropertyDescriptor(Module, prop)) {
    Object.defineProperty(Module, prop, {
      configurable: true,
      set() {
        abort(`Attempt to set \`Module.${prop}\` after it has already been processed.  This can happen, for example, when code is injected via '--post-js' rather than '--pre-js'`);

      }
    });
  }
}

function ignoredModuleProp(prop) {
  if (Object.getOwnPropertyDescriptor(Module, prop)) {
    abort(`\`Module.${prop}\` was supplied but \`${prop}\` not included in INCOMING_MODULE_JS_API`);
  }
}

// forcing the filesystem exports a few things by default
function isExportedByForceFilesystem(name) {
  return name === 'FS_createPath' ||
         name === 'FS_createDataFile' ||
         name === 'FS_createPreloadedFile' ||
         name === 'FS_unlink' ||
         name === 'addRunDependency' ||
         // The old FS has some functionality that WasmFS lacks.
         name === 'FS_createLazyFile' ||
         name === 'FS_createDevice' ||
         name === 'removeRunDependency';
}

/**
 * Intercept access to a global symbol.  This enables us to give informative
 * warnings/errors when folks attempt to use symbols they did not include in
 * their build, or no symbols that no longer exist.
 */
function hookGlobalSymbolAccess(sym, func) {
  if (typeof globalThis != 'undefined' && !Object.getOwnPropertyDescriptor(globalThis, sym)) {
    Object.defineProperty(globalThis, sym, {
      configurable: true,
      get() {
        func();
        return undefined;
      }
    });
  }
}

function missingGlobal(sym, msg) {
  hookGlobalSymbolAccess(sym, () => {
    warnOnce(`\`${sym}\` is not longer defined by emscripten. ${msg}`);
  });
}

missingGlobal('buffer', 'Please use HEAP8.buffer or wasmMemory.buffer');
missingGlobal('asm', 'Please use wasmExports instead');

function missingLibrarySymbol(sym) {
  hookGlobalSymbolAccess(sym, () => {
    // Can't `abort()` here because it would break code that does runtime
    // checks.  e.g. `if (typeof SDL === 'undefined')`.
    var msg = `\`${sym}\` is a library symbol and not included by default; add it to your library.js __deps or to DEFAULT_LIBRARY_FUNCS_TO_INCLUDE on the command line`;
    // DEFAULT_LIBRARY_FUNCS_TO_INCLUDE requires the name as it appears in
    // library.js, which means $name for a JS name with no prefix, or name
    // for a JS name like _name.
    var librarySymbol = sym;
    if (!librarySymbol.startsWith('_')) {
      librarySymbol = '$' + sym;
    }
    msg += ` (e.g. -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE='${librarySymbol}')`;
    if (isExportedByForceFilesystem(sym)) {
      msg += '. Alternatively, forcing filesystem support (-sFORCE_FILESYSTEM) can export this for you';
    }
    warnOnce(msg);
  });

  // Any symbol that is not included from the JS library is also (by definition)
  // not exported on the Module object.
  unexportedRuntimeSymbol(sym);
}

function unexportedRuntimeSymbol(sym) {
  if (!Object.getOwnPropertyDescriptor(Module, sym)) {
    Object.defineProperty(Module, sym, {
      configurable: true,
      get() {
        var msg = `'${sym}' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the Emscripten FAQ)`;
        if (isExportedByForceFilesystem(sym)) {
          msg += '. Alternatively, forcing filesystem support (-sFORCE_FILESYSTEM) can export this for you';
        }
        abort(msg);
      }
    });
  }
}

// end include: runtime_debug.js
// include: memoryprofiler.js
// end include: memoryprofiler.js


function updateMemoryViews() {
  var b = wasmMemory.buffer;
  HEAP8 = new Int8Array(b);
  HEAP16 = new Int16Array(b);
  HEAPU8 = new Uint8Array(b);
  HEAPU16 = new Uint16Array(b);
  HEAP32 = new Int32Array(b);
  HEAPU32 = new Uint32Array(b);
  HEAPF32 = new Float32Array(b);
  HEAPF64 = new Float64Array(b);
  HEAP64 = new BigInt64Array(b);
  HEAPU64 = new BigUint64Array(b);
}

// end include: runtime_shared.js
assert(typeof Int32Array != 'undefined' && typeof Float64Array !== 'undefined' && Int32Array.prototype.subarray != undefined && Int32Array.prototype.set != undefined,
       'JS engine does not provide full typed array support');

function preRun() {
  if (Module['preRun']) {
    if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
    while (Module['preRun'].length) {
      addOnPreRun(Module['preRun'].shift());
    }
  }
  consumedModuleProp('preRun');
  // Begin ATPRERUNS hooks
  callRuntimeCallbacks(onPreRuns);
  // End ATPRERUNS hooks
}

function initRuntime() {
  assert(!runtimeInitialized);
  runtimeInitialized = true;

  checkStackCookie();

  // Begin ATINITS hooks
  if (!Module['noFSInit'] && !FS.initialized) FS.init();
TTY.init();
  // End ATINITS hooks

  wasmExports['__wasm_call_ctors']();

  // Begin ATPOSTCTORS hooks
  FS.ignorePermissions = false;
  // End ATPOSTCTORS hooks
}

function preMain() {
  checkStackCookie();
  // No ATMAINS hooks
}

function postRun() {
  checkStackCookie();
   // PThreads reuse the runtime from the main thread.

  if (Module['postRun']) {
    if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
    while (Module['postRun'].length) {
      addOnPostRun(Module['postRun'].shift());
    }
  }
  consumedModuleProp('postRun');

  // Begin ATPOSTRUNS hooks
  callRuntimeCallbacks(onPostRuns);
  // End ATPOSTRUNS hooks
}

// A counter of dependencies for calling run(). If we need to
// do asynchronous work before running, increment this and
// decrement it. Incrementing must happen in a place like
// Module.preRun (used by emcc to add file preloading).
// Note that you can add dependencies in preRun, even though
// it happens right before run - run will be postponed until
// the dependencies are met.
var runDependencies = 0;
var dependenciesFulfilled = null; // overridden to take different actions when all run dependencies are fulfilled
var runDependencyTracking = {};
var runDependencyWatcher = null;

function getUniqueRunDependency(id) {
  var orig = id;
  while (1) {
    if (!runDependencyTracking[id]) return id;
    id = orig + Math.random();
  }
}

function addRunDependency(id) {
  runDependencies++;

  Module['monitorRunDependencies']?.(runDependencies);

  if (id) {
    assert(!runDependencyTracking[id]);
    runDependencyTracking[id] = 1;
    if (runDependencyWatcher === null && typeof setInterval != 'undefined') {
      // Check for missing dependencies every few seconds
      runDependencyWatcher = setInterval(() => {
        if (ABORT) {
          clearInterval(runDependencyWatcher);
          runDependencyWatcher = null;
          return;
        }
        var shown = false;
        for (var dep in runDependencyTracking) {
          if (!shown) {
            shown = true;
            err('still waiting on run dependencies:');
          }
          err(`dependency: ${dep}`);
        }
        if (shown) {
          err('(end of list)');
        }
      }, 10000);
    }
  } else {
    err('warning: run dependency added without ID');
  }
}

function removeRunDependency(id) {
  runDependencies--;

  Module['monitorRunDependencies']?.(runDependencies);

  if (id) {
    assert(runDependencyTracking[id]);
    delete runDependencyTracking[id];
  } else {
    err('warning: run dependency removed without ID');
  }
  if (runDependencies == 0) {
    if (runDependencyWatcher !== null) {
      clearInterval(runDependencyWatcher);
      runDependencyWatcher = null;
    }
    if (dependenciesFulfilled) {
      var callback = dependenciesFulfilled;
      dependenciesFulfilled = null;
      callback(); // can add another dependenciesFulfilled
    }
  }
}

/** @param {string|number=} what */
function abort(what) {
  Module['onAbort']?.(what);

  what = 'Aborted(' + what + ')';
  // TODO(sbc): Should we remove printing and leave it up to whoever
  // catches the exception?
  err(what);

  ABORT = true;

  // Use a wasm runtime error, because a JS error might be seen as a foreign
  // exception, which means we'd run destructors on it. We need the error to
  // simply make the program stop.
  // FIXME This approach does not work in Wasm EH because it currently does not assume
  // all RuntimeErrors are from traps; it decides whether a RuntimeError is from
  // a trap or not based on a hidden field within the object. So at the moment
  // we don't have a way of throwing a wasm trap from JS. TODO Make a JS API that
  // allows this in the wasm spec.

  // Suppress closure compiler warning here. Closure compiler's builtin extern
  // definition for WebAssembly.RuntimeError claims it takes no arguments even
  // though it can.
  // TODO(https://github.com/google/closure-compiler/pull/3913): Remove if/when upstream closure gets fixed.
  /** @suppress {checkTypes} */
  var e = new WebAssembly.RuntimeError(what);

  // Throw the error whether or not MODULARIZE is set because abort is used
  // in code paths apart from instantiation where an exception is expected
  // to be thrown when abort is called.
  throw e;
}

function createExportWrapper(name, nargs) {
  return (...args) => {
    assert(runtimeInitialized, `native function \`${name}\` called before runtime initialization`);
    var f = wasmExports[name];
    assert(f, `exported native function \`${name}\` not found`);
    // Only assert for too many arguments. Too few can be valid since the missing arguments will be zero filled.
    assert(args.length <= nargs, `native function \`${name}\` called with ${args.length} args but expects ${nargs}`);
    return f(...args);
  };
}

var wasmBinaryFile;

function findWasmBinary() {
    return locateFile('main.wasm');
}

function getBinarySync(file) {
  if (file == wasmBinaryFile && wasmBinary) {
    return new Uint8Array(wasmBinary);
  }
  if (readBinary) {
    return readBinary(file);
  }
  throw 'both async and sync fetching of the wasm failed';
}

async function getWasmBinary(binaryFile) {
  // If we don't have the binary yet, load it asynchronously using readAsync.
  if (!wasmBinary) {
    // Fetch the binary using readAsync
    try {
      var response = await readAsync(binaryFile);
      return new Uint8Array(response);
    } catch {
      // Fall back to getBinarySync below;
    }
  }

  // Otherwise, getBinarySync should be able to get it synchronously
  return getBinarySync(binaryFile);
}

async function instantiateArrayBuffer(binaryFile, imports) {
  try {
    var binary = await getWasmBinary(binaryFile);
    var instance = await WebAssembly.instantiate(binary, imports);
    return instance;
  } catch (reason) {
    err(`failed to asynchronously prepare wasm: ${reason}`);

    // Warn on some common problems.
    if (isFileURI(wasmBinaryFile)) {
      err(`warning: Loading from a file URI (${wasmBinaryFile}) is not supported in most browsers. See https://emscripten.org/docs/getting_started/FAQ.html#how-do-i-run-a-local-webserver-for-testing-why-does-my-program-stall-in-downloading-or-preparing`);
    }
    abort(reason);
  }
}

async function instantiateAsync(binary, binaryFile, imports) {
  if (!binary && typeof WebAssembly.instantiateStreaming == 'function'
      // Don't use streaming for file:// delivered objects in a webview, fetch them synchronously.
      && !isFileURI(binaryFile)
      // Avoid instantiateStreaming() on Node.js environment for now, as while
      // Node.js v18.1.0 implements it, it does not have a full fetch()
      // implementation yet.
      //
      // Reference:
      //   https://github.com/emscripten-core/emscripten/pull/16917
      && !ENVIRONMENT_IS_NODE
     ) {
    try {
      var response = fetch(binaryFile, { credentials: 'same-origin' });
      var instantiationResult = await WebAssembly.instantiateStreaming(response, imports);
      return instantiationResult;
    } catch (reason) {
      // We expect the most common failure cause to be a bad MIME type for the binary,
      // in which case falling back to ArrayBuffer instantiation should work.
      err(`wasm streaming compile failed: ${reason}`);
      err('falling back to ArrayBuffer instantiation');
      // fall back of instantiateArrayBuffer below
    };
  }
  return instantiateArrayBuffer(binaryFile, imports);
}

function getWasmImports() {
  // prepare imports
  return {
    'env': wasmImports,
    'wasi_snapshot_preview1': wasmImports,
  }
}

// Create the wasm instance.
// Receives the wasm imports, returns the exports.
async function createWasm() {
  // Load the wasm module and create an instance of using native support in the JS engine.
  // handle a generated wasm instance, receiving its exports and
  // performing other necessary setup
  /** @param {WebAssembly.Module=} module*/
  function receiveInstance(instance, module) {
    wasmExports = instance.exports;

    

    wasmMemory = wasmExports['memory'];
    
    assert(wasmMemory, 'memory not found in wasm exports');
    updateMemoryViews();

    wasmTable = wasmExports['__indirect_function_table'];
    
    assert(wasmTable, 'table not found in wasm exports');

    removeRunDependency('wasm-instantiate');
    return wasmExports;
  }
  // wait for the pthread pool (if any)
  addRunDependency('wasm-instantiate');

  // Prefer streaming instantiation if available.
  // Async compilation can be confusing when an error on the page overwrites Module
  // (for example, if the order of elements is wrong, and the one defining Module is
  // later), so we save Module and check it later.
  var trueModule = Module;
  function receiveInstantiationResult(result) {
    // 'result' is a ResultObject object which has both the module and instance.
    // receiveInstance() will swap in the exports (to Module.asm) so they can be called
    assert(Module === trueModule, 'the Module object should not be replaced during async compilation - perhaps the order of HTML elements is wrong?');
    trueModule = null;
    // TODO: Due to Closure regression https://github.com/google/closure-compiler/issues/3193, the above line no longer optimizes out down to the following line.
    // When the regression is fixed, can restore the above PTHREADS-enabled path.
    return receiveInstance(result['instance']);
  }

  var info = getWasmImports();

  // User shell pages can write their own Module.instantiateWasm = function(imports, successCallback) callback
  // to manually instantiate the Wasm module themselves. This allows pages to
  // run the instantiation parallel to any other async startup actions they are
  // performing.
  // Also pthreads and wasm workers initialize the wasm instance through this
  // path.
  if (Module['instantiateWasm']) {
    return new Promise((resolve, reject) => {
      try {
        Module['instantiateWasm'](info, (mod, inst) => {
          resolve(receiveInstance(mod, inst));
        });
      } catch(e) {
        err(`Module.instantiateWasm callback failed with error: ${e}`);
        reject(e);
      }
    });
  }

  wasmBinaryFile ??= findWasmBinary();
    var result = await instantiateAsync(wasmBinary, wasmBinaryFile, info);
    var exports = receiveInstantiationResult(result);
    return exports;
}

// end include: preamble.js

// Begin JS library code


  class ExitStatus {
      name = 'ExitStatus';
      constructor(status) {
        this.message = `Program terminated with exit(${status})`;
        this.status = status;
      }
    }

  var callRuntimeCallbacks = (callbacks) => {
      while (callbacks.length > 0) {
        // Pass the module as the first argument.
        callbacks.shift()(Module);
      }
    };
  var onPostRuns = [];
  var addOnPostRun = (cb) => onPostRuns.push(cb);

  var onPreRuns = [];
  var addOnPreRun = (cb) => onPreRuns.push(cb);


  
    /**
     * @param {number} ptr
     * @param {string} type
     */
  function getValue(ptr, type = 'i8') {
    if (type.endsWith('*')) type = '*';
    switch (type) {
      case 'i1': return HEAP8[ptr];
      case 'i8': return HEAP8[ptr];
      case 'i16': return HEAP16[((ptr)>>1)];
      case 'i32': return HEAP32[((ptr)>>2)];
      case 'i64': return HEAP64[((ptr)>>3)];
      case 'float': return HEAPF32[((ptr)>>2)];
      case 'double': return HEAPF64[((ptr)>>3)];
      case '*': return HEAPU32[((ptr)>>2)];
      default: abort(`invalid type for getValue: ${type}`);
    }
  }

  var noExitRuntime = true;

  var ptrToString = (ptr) => {
      assert(typeof ptr === 'number');
      // With CAN_ADDRESS_2GB or MEMORY64, pointers are already unsigned.
      ptr >>>= 0;
      return '0x' + ptr.toString(16).padStart(8, '0');
    };

  
    /**
     * @param {number} ptr
     * @param {number} value
     * @param {string} type
     */
  function setValue(ptr, value, type = 'i8') {
    if (type.endsWith('*')) type = '*';
    switch (type) {
      case 'i1': HEAP8[ptr] = value; break;
      case 'i8': HEAP8[ptr] = value; break;
      case 'i16': HEAP16[((ptr)>>1)] = value; break;
      case 'i32': HEAP32[((ptr)>>2)] = value; break;
      case 'i64': HEAP64[((ptr)>>3)] = BigInt(value); break;
      case 'float': HEAPF32[((ptr)>>2)] = value; break;
      case 'double': HEAPF64[((ptr)>>3)] = value; break;
      case '*': HEAPU32[((ptr)>>2)] = value; break;
      default: abort(`invalid type for setValue: ${type}`);
    }
  }

  var stackRestore = (val) => __emscripten_stack_restore(val);

  var stackSave = () => _emscripten_stack_get_current();

  var warnOnce = (text) => {
      warnOnce.shown ||= {};
      if (!warnOnce.shown[text]) {
        warnOnce.shown[text] = 1;
        if (ENVIRONMENT_IS_NODE) text = 'warning: ' + text;
        err(text);
      }
    };

  var UTF8Decoder = typeof TextDecoder != 'undefined' ? new TextDecoder() : undefined;
  
    /**
     * Given a pointer 'idx' to a null-terminated UTF8-encoded string in the given
     * array that contains uint8 values, returns a copy of that string as a
     * Javascript String object.
     * heapOrArray is either a regular array, or a JavaScript typed array view.
     * @param {number=} idx
     * @param {number=} maxBytesToRead
     * @return {string}
     */
  var UTF8ArrayToString = (heapOrArray, idx = 0, maxBytesToRead = NaN) => {
      var endIdx = idx + maxBytesToRead;
      var endPtr = idx;
      // TextDecoder needs to know the byte length in advance, it doesn't stop on
      // null terminator by itself.  Also, use the length info to avoid running tiny
      // strings through TextDecoder, since .subarray() allocates garbage.
      // (As a tiny code save trick, compare endPtr against endIdx using a negation,
      // so that undefined/NaN means Infinity)
      while (heapOrArray[endPtr] && !(endPtr >= endIdx)) ++endPtr;
  
      // When using conditional TextDecoder, skip it for short strings as the overhead of the native call is not worth it.
      if (endPtr - idx > 16 && heapOrArray.buffer && UTF8Decoder) {
        return UTF8Decoder.decode(heapOrArray.subarray(idx, endPtr));
      }
      var str = '';
      // If building with TextDecoder, we have already computed the string length
      // above, so test loop end condition against that
      while (idx < endPtr) {
        // For UTF8 byte structure, see:
        // http://en.wikipedia.org/wiki/UTF-8#Description
        // https://www.ietf.org/rfc/rfc2279.txt
        // https://tools.ietf.org/html/rfc3629
        var u0 = heapOrArray[idx++];
        if (!(u0 & 0x80)) { str += String.fromCharCode(u0); continue; }
        var u1 = heapOrArray[idx++] & 63;
        if ((u0 & 0xE0) == 0xC0) { str += String.fromCharCode(((u0 & 31) << 6) | u1); continue; }
        var u2 = heapOrArray[idx++] & 63;
        if ((u0 & 0xF0) == 0xE0) {
          u0 = ((u0 & 15) << 12) | (u1 << 6) | u2;
        } else {
          if ((u0 & 0xF8) != 0xF0) warnOnce('Invalid UTF-8 leading byte ' + ptrToString(u0) + ' encountered when deserializing a UTF-8 string in wasm memory to a JS string!');
          u0 = ((u0 & 7) << 18) | (u1 << 12) | (u2 << 6) | (heapOrArray[idx++] & 63);
        }
  
        if (u0 < 0x10000) {
          str += String.fromCharCode(u0);
        } else {
          var ch = u0 - 0x10000;
          str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
        }
      }
      return str;
    };
  
    /**
     * Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the
     * emscripten HEAP, returns a copy of that string as a Javascript String object.
     *
     * @param {number} ptr
     * @param {number=} maxBytesToRead - An optional length that specifies the
     *   maximum number of bytes to read. You can omit this parameter to scan the
     *   string until the first 0 byte. If maxBytesToRead is passed, and the string
     *   at [ptr, ptr+maxBytesToReadr[ contains a null byte in the middle, then the
     *   string will cut short at that byte index (i.e. maxBytesToRead will not
     *   produce a string of exact length [ptr, ptr+maxBytesToRead[) N.B. mixing
     *   frequent uses of UTF8ToString() with and without maxBytesToRead may throw
     *   JS JIT optimizations off, so it is worth to consider consistently using one
     * @return {string}
     */
  var UTF8ToString = (ptr, maxBytesToRead) => {
      assert(typeof ptr == 'number', `UTF8ToString expects a number (got ${typeof ptr})`);
      return ptr ? UTF8ArrayToString(HEAPU8, ptr, maxBytesToRead) : '';
    };
  var ___assert_fail = (condition, filename, line, func) =>
      abort(`Assertion failed: ${UTF8ToString(condition)}, at: ` + [filename ? UTF8ToString(filename) : 'unknown filename', line, func ? UTF8ToString(func) : 'unknown function']);

  var exceptionCaught =  [];
  
  
  
  var uncaughtExceptionCount = 0;
  var ___cxa_begin_catch = (ptr) => {
      var info = new ExceptionInfo(ptr);
      if (!info.get_caught()) {
        info.set_caught(true);
        uncaughtExceptionCount--;
      }
      info.set_rethrown(false);
      exceptionCaught.push(info);
      ___cxa_increment_exception_refcount(ptr);
      return ___cxa_get_exception_ptr(ptr);
    };

  
  var exceptionLast = 0;
  
  
  var ___cxa_end_catch = () => {
      // Clear state flag.
      _setThrew(0, 0);
      assert(exceptionCaught.length > 0);
      // Call destructor if one is registered then clear it.
      var info = exceptionCaught.pop();
  
      ___cxa_decrement_exception_refcount(info.excPtr);
      exceptionLast = 0; // XXX in decRef?
    };

  
  class ExceptionInfo {
      // excPtr - Thrown object pointer to wrap. Metadata pointer is calculated from it.
      constructor(excPtr) {
        this.excPtr = excPtr;
        this.ptr = excPtr - 24;
      }
  
      set_type(type) {
        HEAPU32[(((this.ptr)+(4))>>2)] = type;
      }
  
      get_type() {
        return HEAPU32[(((this.ptr)+(4))>>2)];
      }
  
      set_destructor(destructor) {
        HEAPU32[(((this.ptr)+(8))>>2)] = destructor;
      }
  
      get_destructor() {
        return HEAPU32[(((this.ptr)+(8))>>2)];
      }
  
      set_caught(caught) {
        caught = caught ? 1 : 0;
        HEAP8[(this.ptr)+(12)] = caught;
      }
  
      get_caught() {
        return HEAP8[(this.ptr)+(12)] != 0;
      }
  
      set_rethrown(rethrown) {
        rethrown = rethrown ? 1 : 0;
        HEAP8[(this.ptr)+(13)] = rethrown;
      }
  
      get_rethrown() {
        return HEAP8[(this.ptr)+(13)] != 0;
      }
  
      // Initialize native structure fields. Should be called once after allocated.
      init(type, destructor) {
        this.set_adjusted_ptr(0);
        this.set_type(type);
        this.set_destructor(destructor);
      }
  
      set_adjusted_ptr(adjustedPtr) {
        HEAPU32[(((this.ptr)+(16))>>2)] = adjustedPtr;
      }
  
      get_adjusted_ptr() {
        return HEAPU32[(((this.ptr)+(16))>>2)];
      }
    }
  
  
  var setTempRet0 = (val) => __emscripten_tempret_set(val);
  var findMatchingCatch = (args) => {
      var thrown =
        exceptionLast?.excPtr;
      if (!thrown) {
        // just pass through the null ptr
        setTempRet0(0);
        return 0;
      }
      var info = new ExceptionInfo(thrown);
      info.set_adjusted_ptr(thrown);
      var thrownType = info.get_type();
      if (!thrownType) {
        // just pass through the thrown ptr
        setTempRet0(0);
        return thrown;
      }
  
      // can_catch receives a **, add indirection
      // The different catch blocks are denoted by different types.
      // Due to inheritance, those types may not precisely match the
      // type of the thrown object. Find one which matches, and
      // return the type of the catch block which should be called.
      for (var caughtType of args) {
        if (caughtType === 0 || caughtType === thrownType) {
          // Catch all clause matched or exactly the same type is caught
          break;
        }
        var adjusted_ptr_addr = info.ptr + 16;
        if (___cxa_can_catch(caughtType, thrownType, adjusted_ptr_addr)) {
          setTempRet0(caughtType);
          return thrown;
        }
      }
      setTempRet0(thrownType);
      return thrown;
    };
  var ___cxa_find_matching_catch_2 = () => findMatchingCatch([]);

  var ___cxa_find_matching_catch_3 = (arg0) => findMatchingCatch([arg0]);

  
  
  var ___cxa_rethrow = () => {
      var info = exceptionCaught.pop();
      if (!info) {
        abort('no exception to throw');
      }
      var ptr = info.excPtr;
      if (!info.get_rethrown()) {
        // Only pop if the corresponding push was through rethrow_primary_exception
        exceptionCaught.push(info);
        info.set_rethrown(true);
        info.set_caught(false);
        uncaughtExceptionCount++;
      }
      exceptionLast = new CppException(ptr);
      throw exceptionLast;
    };

  
  
  var ___cxa_throw = (ptr, type, destructor) => {
      var info = new ExceptionInfo(ptr);
      // Initialize ExceptionInfo content after it was allocated in __cxa_allocate_exception.
      info.init(type, destructor);
      exceptionLast = new CppException(ptr);
      uncaughtExceptionCount++;
      throw exceptionLast;
    };

  var ___cxa_uncaught_exceptions = () => uncaughtExceptionCount;

  var ___resumeException = (ptr) => {
      if (!exceptionLast) {
        exceptionLast = new CppException(ptr);
      }
      throw exceptionLast;
    };

  var PATH = {
  isAbs:(path) => path.charAt(0) === '/',
  splitPath:(filename) => {
        var splitPathRe = /^(\/?|)([\s\S]*?)((?:\.{1,2}|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/;
        return splitPathRe.exec(filename).slice(1);
      },
  normalizeArray:(parts, allowAboveRoot) => {
        // if the path tries to go above the root, `up` ends up > 0
        var up = 0;
        for (var i = parts.length - 1; i >= 0; i--) {
          var last = parts[i];
          if (last === '.') {
            parts.splice(i, 1);
          } else if (last === '..') {
            parts.splice(i, 1);
            up++;
          } else if (up) {
            parts.splice(i, 1);
            up--;
          }
        }
        // if the path is allowed to go above the root, restore leading ..s
        if (allowAboveRoot) {
          for (; up; up--) {
            parts.unshift('..');
          }
        }
        return parts;
      },
  normalize:(path) => {
        var isAbsolute = PATH.isAbs(path),
            trailingSlash = path.slice(-1) === '/';
        // Normalize the path
        path = PATH.normalizeArray(path.split('/').filter((p) => !!p), !isAbsolute).join('/');
        if (!path && !isAbsolute) {
          path = '.';
        }
        if (path && trailingSlash) {
          path += '/';
        }
        return (isAbsolute ? '/' : '') + path;
      },
  dirname:(path) => {
        var result = PATH.splitPath(path),
            root = result[0],
            dir = result[1];
        if (!root && !dir) {
          // No dirname whatsoever
          return '.';
        }
        if (dir) {
          // It has a dirname, strip trailing slash
          dir = dir.slice(0, -1);
        }
        return root + dir;
      },
  basename:(path) => path && path.match(/([^\/]+|\/)\/*$/)[1],
  join:(...paths) => PATH.normalize(paths.join('/')),
  join2:(l, r) => PATH.normalize(l + '/' + r),
  };
  
  var initRandomFill = () => {
      // This block is not needed on v19+ since crypto.getRandomValues is builtin
      if (ENVIRONMENT_IS_NODE) {
        var nodeCrypto = require('crypto');
        return (view) => nodeCrypto.randomFillSync(view);
      }
  
      return (view) => crypto.getRandomValues(view);
    };
  var randomFill = (view) => {
      // Lazily init on the first invocation.
      (randomFill = initRandomFill())(view);
    };
  
  
  
  var PATH_FS = {
  resolve:(...args) => {
        var resolvedPath = '',
          resolvedAbsolute = false;
        for (var i = args.length - 1; i >= -1 && !resolvedAbsolute; i--) {
          var path = (i >= 0) ? args[i] : FS.cwd();
          // Skip empty and invalid entries
          if (typeof path != 'string') {
            throw new TypeError('Arguments to path.resolve must be strings');
          } else if (!path) {
            return ''; // an invalid portion invalidates the whole thing
          }
          resolvedPath = path + '/' + resolvedPath;
          resolvedAbsolute = PATH.isAbs(path);
        }
        // At this point the path should be resolved to a full absolute path, but
        // handle relative paths to be safe (might happen when process.cwd() fails)
        resolvedPath = PATH.normalizeArray(resolvedPath.split('/').filter((p) => !!p), !resolvedAbsolute).join('/');
        return ((resolvedAbsolute ? '/' : '') + resolvedPath) || '.';
      },
  relative:(from, to) => {
        from = PATH_FS.resolve(from).slice(1);
        to = PATH_FS.resolve(to).slice(1);
        function trim(arr) {
          var start = 0;
          for (; start < arr.length; start++) {
            if (arr[start] !== '') break;
          }
          var end = arr.length - 1;
          for (; end >= 0; end--) {
            if (arr[end] !== '') break;
          }
          if (start > end) return [];
          return arr.slice(start, end - start + 1);
        }
        var fromParts = trim(from.split('/'));
        var toParts = trim(to.split('/'));
        var length = Math.min(fromParts.length, toParts.length);
        var samePartsLength = length;
        for (var i = 0; i < length; i++) {
          if (fromParts[i] !== toParts[i]) {
            samePartsLength = i;
            break;
          }
        }
        var outputParts = [];
        for (var i = samePartsLength; i < fromParts.length; i++) {
          outputParts.push('..');
        }
        outputParts = outputParts.concat(toParts.slice(samePartsLength));
        return outputParts.join('/');
      },
  };
  
  
  
  var FS_stdin_getChar_buffer = [];
  
  var lengthBytesUTF8 = (str) => {
      var len = 0;
      for (var i = 0; i < str.length; ++i) {
        // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code
        // unit, not a Unicode code point of the character! So decode
        // UTF16->UTF32->UTF8.
        // See http://unicode.org/faq/utf_bom.html#utf16-3
        var c = str.charCodeAt(i); // possibly a lead surrogate
        if (c <= 0x7F) {
          len++;
        } else if (c <= 0x7FF) {
          len += 2;
        } else if (c >= 0xD800 && c <= 0xDFFF) {
          len += 4; ++i;
        } else {
          len += 3;
        }
      }
      return len;
    };
  
  var stringToUTF8Array = (str, heap, outIdx, maxBytesToWrite) => {
      assert(typeof str === 'string', `stringToUTF8Array expects a string (got ${typeof str})`);
      // Parameter maxBytesToWrite is not optional. Negative values, 0, null,
      // undefined and false each don't write out any bytes.
      if (!(maxBytesToWrite > 0))
        return 0;
  
      var startIdx = outIdx;
      var endIdx = outIdx + maxBytesToWrite - 1; // -1 for string null terminator.
      for (var i = 0; i < str.length; ++i) {
        // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code
        // unit, not a Unicode code point of the character! So decode
        // UTF16->UTF32->UTF8.
        // See http://unicode.org/faq/utf_bom.html#utf16-3
        // For UTF8 byte structure, see http://en.wikipedia.org/wiki/UTF-8#Description
        // and https://www.ietf.org/rfc/rfc2279.txt
        // and https://tools.ietf.org/html/rfc3629
        var u = str.charCodeAt(i); // possibly a lead surrogate
        if (u >= 0xD800 && u <= 0xDFFF) {
          var u1 = str.charCodeAt(++i);
          u = 0x10000 + ((u & 0x3FF) << 10) | (u1 & 0x3FF);
        }
        if (u <= 0x7F) {
          if (outIdx >= endIdx) break;
          heap[outIdx++] = u;
        } else if (u <= 0x7FF) {
          if (outIdx + 1 >= endIdx) break;
          heap[outIdx++] = 0xC0 | (u >> 6);
          heap[outIdx++] = 0x80 | (u & 63);
        } else if (u <= 0xFFFF) {
          if (outIdx + 2 >= endIdx) break;
          heap[outIdx++] = 0xE0 | (u >> 12);
          heap[outIdx++] = 0x80 | ((u >> 6) & 63);
          heap[outIdx++] = 0x80 | (u & 63);
        } else {
          if (outIdx + 3 >= endIdx) break;
          if (u > 0x10FFFF) warnOnce('Invalid Unicode code point ' + ptrToString(u) + ' encountered when serializing a JS string to a UTF-8 string in wasm memory! (Valid unicode code points should be in range 0-0x10FFFF).');
          heap[outIdx++] = 0xF0 | (u >> 18);
          heap[outIdx++] = 0x80 | ((u >> 12) & 63);
          heap[outIdx++] = 0x80 | ((u >> 6) & 63);
          heap[outIdx++] = 0x80 | (u & 63);
        }
      }
      // Null-terminate the pointer to the buffer.
      heap[outIdx] = 0;
      return outIdx - startIdx;
    };
  /** @type {function(string, boolean=, number=)} */
  var intArrayFromString = (stringy, dontAddNull, length) => {
      var len = length > 0 ? length : lengthBytesUTF8(stringy)+1;
      var u8array = new Array(len);
      var numBytesWritten = stringToUTF8Array(stringy, u8array, 0, u8array.length);
      if (dontAddNull) u8array.length = numBytesWritten;
      return u8array;
    };
  var FS_stdin_getChar = () => {
      if (!FS_stdin_getChar_buffer.length) {
        var result = null;
        if (ENVIRONMENT_IS_NODE) {
          // we will read data by chunks of BUFSIZE
          var BUFSIZE = 256;
          var buf = Buffer.alloc(BUFSIZE);
          var bytesRead = 0;
  
          // For some reason we must suppress a closure warning here, even though
          // fd definitely exists on process.stdin, and is even the proper way to
          // get the fd of stdin,
          // https://github.com/nodejs/help/issues/2136#issuecomment-523649904
          // This started to happen after moving this logic out of library_tty.js,
          // so it is related to the surrounding code in some unclear manner.
          /** @suppress {missingProperties} */
          var fd = process.stdin.fd;
  
          try {
            bytesRead = fs.readSync(fd, buf, 0, BUFSIZE);
          } catch(e) {
            // Cross-platform differences: on Windows, reading EOF throws an
            // exception, but on other OSes, reading EOF returns 0. Uniformize
            // behavior by treating the EOF exception to return 0.
            if (e.toString().includes('EOF')) bytesRead = 0;
            else throw e;
          }
  
          if (bytesRead > 0) {
            result = buf.slice(0, bytesRead).toString('utf-8');
          }
        } else
        if (typeof window != 'undefined' &&
          typeof window.prompt == 'function') {
          // Browser.
          result = window.prompt('Input: ');  // returns null on cancel
          if (result !== null) {
            result += '\n';
          }
        } else
        {}
        if (!result) {
          return null;
        }
        FS_stdin_getChar_buffer = intArrayFromString(result, true);
      }
      return FS_stdin_getChar_buffer.shift();
    };
  var TTY = {
  ttys:[],
  init() {
        // https://github.com/emscripten-core/emscripten/pull/1555
        // if (ENVIRONMENT_IS_NODE) {
        //   // currently, FS.init does not distinguish if process.stdin is a file or TTY
        //   // device, it always assumes it's a TTY device. because of this, we're forcing
        //   // process.stdin to UTF8 encoding to at least make stdin reading compatible
        //   // with text files until FS.init can be refactored.
        //   process.stdin.setEncoding('utf8');
        // }
      },
  shutdown() {
        // https://github.com/emscripten-core/emscripten/pull/1555
        // if (ENVIRONMENT_IS_NODE) {
        //   // inolen: any idea as to why node -e 'process.stdin.read()' wouldn't exit immediately (with process.stdin being a tty)?
        //   // isaacs: because now it's reading from the stream, you've expressed interest in it, so that read() kicks off a _read() which creates a ReadReq operation
        //   // inolen: I thought read() in that case was a synchronous operation that just grabbed some amount of buffered data if it exists?
        //   // isaacs: it is. but it also triggers a _read() call, which calls readStart() on the handle
        //   // isaacs: do process.stdin.pause() and i'd think it'd probably close the pending call
        //   process.stdin.pause();
        // }
      },
  register(dev, ops) {
        TTY.ttys[dev] = { input: [], output: [], ops: ops };
        FS.registerDevice(dev, TTY.stream_ops);
      },
  stream_ops:{
  open(stream) {
          var tty = TTY.ttys[stream.node.rdev];
          if (!tty) {
            throw new FS.ErrnoError(43);
          }
          stream.tty = tty;
          stream.seekable = false;
        },
  close(stream) {
          // flush any pending line data
          stream.tty.ops.fsync(stream.tty);
        },
  fsync(stream) {
          stream.tty.ops.fsync(stream.tty);
        },
  read(stream, buffer, offset, length, pos /* ignored */) {
          if (!stream.tty || !stream.tty.ops.get_char) {
            throw new FS.ErrnoError(60);
          }
          var bytesRead = 0;
          for (var i = 0; i < length; i++) {
            var result;
            try {
              result = stream.tty.ops.get_char(stream.tty);
            } catch (e) {
              throw new FS.ErrnoError(29);
            }
            if (result === undefined && bytesRead === 0) {
              throw new FS.ErrnoError(6);
            }
            if (result === null || result === undefined) break;
            bytesRead++;
            buffer[offset+i] = result;
          }
          if (bytesRead) {
            stream.node.atime = Date.now();
          }
          return bytesRead;
        },
  write(stream, buffer, offset, length, pos) {
          if (!stream.tty || !stream.tty.ops.put_char) {
            throw new FS.ErrnoError(60);
          }
          try {
            for (var i = 0; i < length; i++) {
              stream.tty.ops.put_char(stream.tty, buffer[offset+i]);
            }
          } catch (e) {
            throw new FS.ErrnoError(29);
          }
          if (length) {
            stream.node.mtime = stream.node.ctime = Date.now();
          }
          return i;
        },
  },
  default_tty_ops:{
  get_char(tty) {
          return FS_stdin_getChar();
        },
  put_char(tty, val) {
          if (val === null || val === 10) {
            out(UTF8ArrayToString(tty.output));
            tty.output = [];
          } else {
            if (val != 0) tty.output.push(val); // val == 0 would cut text output off in the middle.
          }
        },
  fsync(tty) {
          if (tty.output?.length > 0) {
            out(UTF8ArrayToString(tty.output));
            tty.output = [];
          }
        },
  ioctl_tcgets(tty) {
          // typical setting
          return {
            c_iflag: 25856,
            c_oflag: 5,
            c_cflag: 191,
            c_lflag: 35387,
            c_cc: [
              0x03, 0x1c, 0x7f, 0x15, 0x04, 0x00, 0x01, 0x00, 0x11, 0x13, 0x1a, 0x00,
              0x12, 0x0f, 0x17, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ]
          };
        },
  ioctl_tcsets(tty, optional_actions, data) {
          // currently just ignore
          return 0;
        },
  ioctl_tiocgwinsz(tty) {
          return [24, 80];
        },
  },
  default_tty1_ops:{
  put_char(tty, val) {
          if (val === null || val === 10) {
            err(UTF8ArrayToString(tty.output));
            tty.output = [];
          } else {
            if (val != 0) tty.output.push(val);
          }
        },
  fsync(tty) {
          if (tty.output?.length > 0) {
            err(UTF8ArrayToString(tty.output));
            tty.output = [];
          }
        },
  },
  };
  
  
  var zeroMemory = (ptr, size) => HEAPU8.fill(0, ptr, ptr + size);
  
  var alignMemory = (size, alignment) => {
      assert(alignment, "alignment argument is required");
      return Math.ceil(size / alignment) * alignment;
    };
  var mmapAlloc = (size) => {
      size = alignMemory(size, 65536);
      var ptr = _emscripten_builtin_memalign(65536, size);
      if (ptr) zeroMemory(ptr, size);
      return ptr;
    };
  var MEMFS = {
  ops_table:null,
  mount(mount) {
        return MEMFS.createNode(null, '/', 16895, 0);
      },
  createNode(parent, name, mode, dev) {
        if (FS.isBlkdev(mode) || FS.isFIFO(mode)) {
          // no supported
          throw new FS.ErrnoError(63);
        }
        MEMFS.ops_table ||= {
          dir: {
            node: {
              getattr: MEMFS.node_ops.getattr,
              setattr: MEMFS.node_ops.setattr,
              lookup: MEMFS.node_ops.lookup,
              mknod: MEMFS.node_ops.mknod,
              rename: MEMFS.node_ops.rename,
              unlink: MEMFS.node_ops.unlink,
              rmdir: MEMFS.node_ops.rmdir,
              readdir: MEMFS.node_ops.readdir,
              symlink: MEMFS.node_ops.symlink
            },
            stream: {
              llseek: MEMFS.stream_ops.llseek
            }
          },
          file: {
            node: {
              getattr: MEMFS.node_ops.getattr,
              setattr: MEMFS.node_ops.setattr
            },
            stream: {
              llseek: MEMFS.stream_ops.llseek,
              read: MEMFS.stream_ops.read,
              write: MEMFS.stream_ops.write,
              mmap: MEMFS.stream_ops.mmap,
              msync: MEMFS.stream_ops.msync
            }
          },
          link: {
            node: {
              getattr: MEMFS.node_ops.getattr,
              setattr: MEMFS.node_ops.setattr,
              readlink: MEMFS.node_ops.readlink
            },
            stream: {}
          },
          chrdev: {
            node: {
              getattr: MEMFS.node_ops.getattr,
              setattr: MEMFS.node_ops.setattr
            },
            stream: FS.chrdev_stream_ops
          }
        };
        var node = FS.createNode(parent, name, mode, dev);
        if (FS.isDir(node.mode)) {
          node.node_ops = MEMFS.ops_table.dir.node;
          node.stream_ops = MEMFS.ops_table.dir.stream;
          node.contents = {};
        } else if (FS.isFile(node.mode)) {
          node.node_ops = MEMFS.ops_table.file.node;
          node.stream_ops = MEMFS.ops_table.file.stream;
          node.usedBytes = 0; // The actual number of bytes used in the typed array, as opposed to contents.length which gives the whole capacity.
          // When the byte data of the file is populated, this will point to either a typed array, or a normal JS array. Typed arrays are preferred
          // for performance, and used by default. However, typed arrays are not resizable like normal JS arrays are, so there is a small disk size
          // penalty involved for appending file writes that continuously grow a file similar to std::vector capacity vs used -scheme.
          node.contents = null; 
        } else if (FS.isLink(node.mode)) {
          node.node_ops = MEMFS.ops_table.link.node;
          node.stream_ops = MEMFS.ops_table.link.stream;
        } else if (FS.isChrdev(node.mode)) {
          node.node_ops = MEMFS.ops_table.chrdev.node;
          node.stream_ops = MEMFS.ops_table.chrdev.stream;
        }
        node.atime = node.mtime = node.ctime = Date.now();
        // add the new node to the parent
        if (parent) {
          parent.contents[name] = node;
          parent.atime = parent.mtime = parent.ctime = node.atime;
        }
        return node;
      },
  getFileDataAsTypedArray(node) {
        if (!node.contents) return new Uint8Array(0);
        if (node.contents.subarray) return node.contents.subarray(0, node.usedBytes); // Make sure to not return excess unused bytes.
        return new Uint8Array(node.contents);
      },
  expandFileStorage(node, newCapacity) {
        var prevCapacity = node.contents ? node.contents.length : 0;
        if (prevCapacity >= newCapacity) return; // No need to expand, the storage was already large enough.
        // Don't expand strictly to the given requested limit if it's only a very small increase, but instead geometrically grow capacity.
        // For small filesizes (<1MB), perform size*2 geometric increase, but for large sizes, do a much more conservative size*1.125 increase to
        // avoid overshooting the allocation cap by a very large margin.
        var CAPACITY_DOUBLING_MAX = 1024 * 1024;
        newCapacity = Math.max(newCapacity, (prevCapacity * (prevCapacity < CAPACITY_DOUBLING_MAX ? 2.0 : 1.125)) >>> 0);
        if (prevCapacity != 0) newCapacity = Math.max(newCapacity, 256); // At minimum allocate 256b for each file when expanding.
        var oldContents = node.contents;
        node.contents = new Uint8Array(newCapacity); // Allocate new storage.
        if (node.usedBytes > 0) node.contents.set(oldContents.subarray(0, node.usedBytes), 0); // Copy old data over to the new storage.
      },
  resizeFileStorage(node, newSize) {
        if (node.usedBytes == newSize) return;
        if (newSize == 0) {
          node.contents = null; // Fully decommit when requesting a resize to zero.
          node.usedBytes = 0;
        } else {
          var oldContents = node.contents;
          node.contents = new Uint8Array(newSize); // Allocate new storage.
          if (oldContents) {
            node.contents.set(oldContents.subarray(0, Math.min(newSize, node.usedBytes))); // Copy old data over to the new storage.
          }
          node.usedBytes = newSize;
        }
      },
  node_ops:{
  getattr(node) {
          var attr = {};
          // device numbers reuse inode numbers.
          attr.dev = FS.isChrdev(node.mode) ? node.id : 1;
          attr.ino = node.id;
          attr.mode = node.mode;
          attr.nlink = 1;
          attr.uid = 0;
          attr.gid = 0;
          attr.rdev = node.rdev;
          if (FS.isDir(node.mode)) {
            attr.size = 4096;
          } else if (FS.isFile(node.mode)) {
            attr.size = node.usedBytes;
          } else if (FS.isLink(node.mode)) {
            attr.size = node.link.length;
          } else {
            attr.size = 0;
          }
          attr.atime = new Date(node.atime);
          attr.mtime = new Date(node.mtime);
          attr.ctime = new Date(node.ctime);
          // NOTE: In our implementation, st_blocks = Math.ceil(st_size/st_blksize),
          //       but this is not required by the standard.
          attr.blksize = 4096;
          attr.blocks = Math.ceil(attr.size / attr.blksize);
          return attr;
        },
  setattr(node, attr) {
          for (const key of ["mode", "atime", "mtime", "ctime"]) {
            if (attr[key] != null) {
              node[key] = attr[key];
            }
          }
          if (attr.size !== undefined) {
            MEMFS.resizeFileStorage(node, attr.size);
          }
        },
  lookup(parent, name) {
          throw new FS.ErrnoError(44);
        },
  mknod(parent, name, mode, dev) {
          return MEMFS.createNode(parent, name, mode, dev);
        },
  rename(old_node, new_dir, new_name) {
          var new_node;
          try {
            new_node = FS.lookupNode(new_dir, new_name);
          } catch (e) {}
          if (new_node) {
            if (FS.isDir(old_node.mode)) {
              // if we're overwriting a directory at new_name, make sure it's empty.
              for (var i in new_node.contents) {
                throw new FS.ErrnoError(55);
              }
            }
            FS.hashRemoveNode(new_node);
          }
          // do the internal rewiring
          delete old_node.parent.contents[old_node.name];
          new_dir.contents[new_name] = old_node;
          old_node.name = new_name;
          new_dir.ctime = new_dir.mtime = old_node.parent.ctime = old_node.parent.mtime = Date.now();
        },
  unlink(parent, name) {
          delete parent.contents[name];
          parent.ctime = parent.mtime = Date.now();
        },
  rmdir(parent, name) {
          var node = FS.lookupNode(parent, name);
          for (var i in node.contents) {
            throw new FS.ErrnoError(55);
          }
          delete parent.contents[name];
          parent.ctime = parent.mtime = Date.now();
        },
  readdir(node) {
          return ['.', '..', ...Object.keys(node.contents)];
        },
  symlink(parent, newname, oldpath) {
          var node = MEMFS.createNode(parent, newname, 0o777 | 40960, 0);
          node.link = oldpath;
          return node;
        },
  readlink(node) {
          if (!FS.isLink(node.mode)) {
            throw new FS.ErrnoError(28);
          }
          return node.link;
        },
  },
  stream_ops:{
  read(stream, buffer, offset, length, position) {
          var contents = stream.node.contents;
          if (position >= stream.node.usedBytes) return 0;
          var size = Math.min(stream.node.usedBytes - position, length);
          assert(size >= 0);
          if (size > 8 && contents.subarray) { // non-trivial, and typed array
            buffer.set(contents.subarray(position, position + size), offset);
          } else {
            for (var i = 0; i < size; i++) buffer[offset + i] = contents[position + i];
          }
          return size;
        },
  write(stream, buffer, offset, length, position, canOwn) {
          // The data buffer should be a typed array view
          assert(!(buffer instanceof ArrayBuffer));
          // If the buffer is located in main memory (HEAP), and if
          // memory can grow, we can't hold on to references of the
          // memory buffer, as they may get invalidated. That means we
          // need to do copy its contents.
          if (buffer.buffer === HEAP8.buffer) {
            canOwn = false;
          }
  
          if (!length) return 0;
          var node = stream.node;
          node.mtime = node.ctime = Date.now();
  
          if (buffer.subarray && (!node.contents || node.contents.subarray)) { // This write is from a typed array to a typed array?
            if (canOwn) {
              assert(position === 0, 'canOwn must imply no weird position inside the file');
              node.contents = buffer.subarray(offset, offset + length);
              node.usedBytes = length;
              return length;
            } else if (node.usedBytes === 0 && position === 0) { // If this is a simple first write to an empty file, do a fast set since we don't need to care about old data.
              node.contents = buffer.slice(offset, offset + length);
              node.usedBytes = length;
              return length;
            } else if (position + length <= node.usedBytes) { // Writing to an already allocated and used subrange of the file?
              node.contents.set(buffer.subarray(offset, offset + length), position);
              return length;
            }
          }
  
          // Appending to an existing file and we need to reallocate, or source data did not come as a typed array.
          MEMFS.expandFileStorage(node, position+length);
          if (node.contents.subarray && buffer.subarray) {
            // Use typed array write which is available.
            node.contents.set(buffer.subarray(offset, offset + length), position);
          } else {
            for (var i = 0; i < length; i++) {
             node.contents[position + i] = buffer[offset + i]; // Or fall back to manual write if not.
            }
          }
          node.usedBytes = Math.max(node.usedBytes, position + length);
          return length;
        },
  llseek(stream, offset, whence) {
          var position = offset;
          if (whence === 1) {
            position += stream.position;
          } else if (whence === 2) {
            if (FS.isFile(stream.node.mode)) {
              position += stream.node.usedBytes;
            }
          }
          if (position < 0) {
            throw new FS.ErrnoError(28);
          }
          return position;
        },
  mmap(stream, length, position, prot, flags) {
          if (!FS.isFile(stream.node.mode)) {
            throw new FS.ErrnoError(43);
          }
          var ptr;
          var allocated;
          var contents = stream.node.contents;
          // Only make a new copy when MAP_PRIVATE is specified.
          if (!(flags & 2) && contents && contents.buffer === HEAP8.buffer) {
            // We can't emulate MAP_SHARED when the file is not backed by the
            // buffer we're mapping to (e.g. the HEAP buffer).
            allocated = false;
            ptr = contents.byteOffset;
          } else {
            allocated = true;
            ptr = mmapAlloc(length);
            if (!ptr) {
              throw new FS.ErrnoError(48);
            }
            if (contents) {
              // Try to avoid unnecessary slices.
              if (position > 0 || position + length < contents.length) {
                if (contents.subarray) {
                  contents = contents.subarray(position, position + length);
                } else {
                  contents = Array.prototype.slice.call(contents, position, position + length);
                }
              }
              HEAP8.set(contents, ptr);
            }
          }
          return { ptr, allocated };
        },
  msync(stream, buffer, offset, length, mmapFlags) {
          MEMFS.stream_ops.write(stream, buffer, 0, length, offset, false);
          // should we check if bytesWritten and length are the same?
          return 0;
        },
  },
  };
  
  var asyncLoad = async (url) => {
      var arrayBuffer = await readAsync(url);
      assert(arrayBuffer, `Loading data file "${url}" failed (no arrayBuffer).`);
      return new Uint8Array(arrayBuffer);
    };
  
  
  var FS_createDataFile = (...args) => FS.createDataFile(...args);
  
  var preloadPlugins = [];
  var FS_handledByPreloadPlugin = (byteArray, fullname, finish, onerror) => {
      // Ensure plugins are ready.
      if (typeof Browser != 'undefined') Browser.init();
  
      var handled = false;
      preloadPlugins.forEach((plugin) => {
        if (handled) return;
        if (plugin['canHandle'](fullname)) {
          plugin['handle'](byteArray, fullname, finish, onerror);
          handled = true;
        }
      });
      return handled;
    };
  var FS_createPreloadedFile = (parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish) => {
      // TODO we should allow people to just pass in a complete filename instead
      // of parent and name being that we just join them anyways
      var fullname = name ? PATH_FS.resolve(PATH.join2(parent, name)) : parent;
      var dep = getUniqueRunDependency(`cp ${fullname}`); // might have several active requests for the same fullname
      function processData(byteArray) {
        function finish(byteArray) {
          preFinish?.();
          if (!dontCreateFile) {
            FS_createDataFile(parent, name, byteArray, canRead, canWrite, canOwn);
          }
          onload?.();
          removeRunDependency(dep);
        }
        if (FS_handledByPreloadPlugin(byteArray, fullname, finish, () => {
          onerror?.();
          removeRunDependency(dep);
        })) {
          return;
        }
        finish(byteArray);
      }
      addRunDependency(dep);
      if (typeof url == 'string') {
        asyncLoad(url).then(processData, onerror);
      } else {
        processData(url);
      }
    };
  
  var FS_modeStringToFlags = (str) => {
      var flagModes = {
        'r': 0,
        'r+': 2,
        'w': 512 | 64 | 1,
        'w+': 512 | 64 | 2,
        'a': 1024 | 64 | 1,
        'a+': 1024 | 64 | 2,
      };
      var flags = flagModes[str];
      if (typeof flags == 'undefined') {
        throw new Error(`Unknown file open mode: ${str}`);
      }
      return flags;
    };
  
  var FS_getMode = (canRead, canWrite) => {
      var mode = 0;
      if (canRead) mode |= 292 | 73;
      if (canWrite) mode |= 146;
      return mode;
    };
  
  
  
  
  
  
  var IDBFS = {
  dbs:{
  },
  indexedDB:() => {
        if (typeof indexedDB != 'undefined') return indexedDB;
        var ret = null;
        if (typeof window == 'object') ret = window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
        assert(ret, 'IDBFS used, but indexedDB not supported');
        return ret;
      },
  DB_VERSION:21,
  DB_STORE_NAME:"FILE_DATA",
  queuePersist:(mount) => {
        function onPersistComplete() {
          if (mount.idbPersistState === 'again') startPersist(); // If a new sync request has appeared in between, kick off a new sync
          else mount.idbPersistState = 0; // Otherwise reset sync state back to idle to wait for a new sync later
        }
        function startPersist() {
          mount.idbPersistState = 'idb'; // Mark that we are currently running a sync operation
          IDBFS.syncfs(mount, /*populate:*/false, onPersistComplete);
        }
  
        if (!mount.idbPersistState) {
          // Programs typically write/copy/move multiple files in the in-memory
          // filesystem within a single app frame, so when a filesystem sync
          // command is triggered, do not start it immediately, but only after
          // the current frame is finished. This way all the modified files
          // inside the main loop tick will be batched up to the same sync.
          mount.idbPersistState = setTimeout(startPersist, 0);
        } else if (mount.idbPersistState === 'idb') {
          // There is an active IndexedDB sync operation in-flight, but we now
          // have accumulated more files to sync. We should therefore queue up
          // a new sync after the current one finishes so that all writes
          // will be properly persisted.
          mount.idbPersistState = 'again';
        }
      },
  mount:(mount) => {
        // reuse core MEMFS functionality
        var mnt = MEMFS.mount(mount);
        // If the automatic IDBFS persistence option has been selected, then automatically persist
        // all modifications to the filesystem as they occur.
        if (mount?.opts?.autoPersist) {
          mnt.idbPersistState = 0; // IndexedDB sync starts in idle state
          var memfs_node_ops = mnt.node_ops;
          mnt.node_ops = {...mnt.node_ops}; // Clone node_ops to inject write tracking
          mnt.node_ops.mknod = (parent, name, mode, dev) => {
            var node = memfs_node_ops.mknod(parent, name, mode, dev);
            // Propagate injected node_ops to the newly created child node
            node.node_ops = mnt.node_ops;
            // Remember for each IDBFS node which IDBFS mount point they came from so we know which mount to persist on modification.
            node.idbfs_mount = mnt.mount;
            // Remember original MEMFS stream_ops for this node
            node.memfs_stream_ops = node.stream_ops;
            // Clone stream_ops to inject write tracking
            node.stream_ops = {...node.stream_ops};
  
            // Track all file writes
            node.stream_ops.write = (stream, buffer, offset, length, position, canOwn) => {
              // This file has been modified, we must persist IndexedDB when this file closes
              stream.node.isModified = true;
              return node.memfs_stream_ops.write(stream, buffer, offset, length, position, canOwn);
            };
  
            // Persist IndexedDB on file close
            node.stream_ops.close = (stream) => {
              var n = stream.node;
              if (n.isModified) {
                IDBFS.queuePersist(n.idbfs_mount);
                n.isModified = false;
              }
              if (n.memfs_stream_ops.close) return n.memfs_stream_ops.close(stream);
            };
  
            return node;
          };
          // Also kick off persisting the filesystem on other operations that modify the filesystem.
          mnt.node_ops.mkdir   = (...args) => (IDBFS.queuePersist(mnt.mount), memfs_node_ops.mkdir(...args));
          mnt.node_ops.rmdir   = (...args) => (IDBFS.queuePersist(mnt.mount), memfs_node_ops.rmdir(...args));
          mnt.node_ops.symlink = (...args) => (IDBFS.queuePersist(mnt.mount), memfs_node_ops.symlink(...args));
          mnt.node_ops.unlink  = (...args) => (IDBFS.queuePersist(mnt.mount), memfs_node_ops.unlink(...args));
          mnt.node_ops.rename  = (...args) => (IDBFS.queuePersist(mnt.mount), memfs_node_ops.rename(...args));
        }
        return mnt;
      },
  syncfs:(mount, populate, callback) => {
        IDBFS.getLocalSet(mount, (err, local) => {
          if (err) return callback(err);
  
          IDBFS.getRemoteSet(mount, (err, remote) => {
            if (err) return callback(err);
  
            var src = populate ? remote : local;
            var dst = populate ? local : remote;
  
            IDBFS.reconcile(src, dst, callback);
          });
        });
      },
  quit:() => {
        Object.values(IDBFS.dbs).forEach((value) => value.close());
        IDBFS.dbs = {};
      },
  getDB:(name, callback) => {
        // check the cache first
        var db = IDBFS.dbs[name];
        if (db) {
          return callback(null, db);
        }
  
        var req;
        try {
          req = IDBFS.indexedDB().open(name, IDBFS.DB_VERSION);
        } catch (e) {
          return callback(e);
        }
        if (!req) {
          return callback("Unable to connect to IndexedDB");
        }
        req.onupgradeneeded = (e) => {
          var db = /** @type {IDBDatabase} */ (e.target.result);
          var transaction = e.target.transaction;
  
          var fileStore;
  
          if (db.objectStoreNames.contains(IDBFS.DB_STORE_NAME)) {
            fileStore = transaction.objectStore(IDBFS.DB_STORE_NAME);
          } else {
            fileStore = db.createObjectStore(IDBFS.DB_STORE_NAME);
          }
  
          if (!fileStore.indexNames.contains('timestamp')) {
            fileStore.createIndex('timestamp', 'timestamp', { unique: false });
          }
        };
        req.onsuccess = () => {
          db = /** @type {IDBDatabase} */ (req.result);
  
          // add to the cache
          IDBFS.dbs[name] = db;
          callback(null, db);
        };
        req.onerror = (e) => {
          callback(e.target.error);
          e.preventDefault();
        };
      },
  getLocalSet:(mount, callback) => {
        var entries = {};
  
        function isRealDir(p) {
          return p !== '.' && p !== '..';
        };
        function toAbsolute(root) {
          return (p) => PATH.join2(root, p);
        };
  
        var check = FS.readdir(mount.mountpoint).filter(isRealDir).map(toAbsolute(mount.mountpoint));
  
        while (check.length) {
          var path = check.pop();
          var stat;
  
          try {
            stat = FS.stat(path);
          } catch (e) {
            return callback(e);
          }
  
          if (FS.isDir(stat.mode)) {
            check.push(...FS.readdir(path).filter(isRealDir).map(toAbsolute(path)));
          }
  
          entries[path] = { 'timestamp': stat.mtime };
        }
  
        return callback(null, { type: 'local', entries: entries });
      },
  getRemoteSet:(mount, callback) => {
        var entries = {};
  
        IDBFS.getDB(mount.mountpoint, (err, db) => {
          if (err) return callback(err);
  
          try {
            var transaction = db.transaction([IDBFS.DB_STORE_NAME], 'readonly');
            transaction.onerror = (e) => {
              callback(e.target.error);
              e.preventDefault();
            };
  
            var store = transaction.objectStore(IDBFS.DB_STORE_NAME);
            var index = store.index('timestamp');
  
            index.openKeyCursor().onsuccess = (event) => {
              var cursor = event.target.result;
  
              if (!cursor) {
                return callback(null, { type: 'remote', db, entries });
              }
  
              entries[cursor.primaryKey] = { 'timestamp': cursor.key };
  
              cursor.continue();
            };
          } catch (e) {
            return callback(e);
          }
        });
      },
  loadLocalEntry:(path, callback) => {
        var stat, node;
  
        try {
          var lookup = FS.lookupPath(path);
          node = lookup.node;
          stat = FS.stat(path);
        } catch (e) {
          return callback(e);
        }
  
        if (FS.isDir(stat.mode)) {
          return callback(null, { 'timestamp': stat.mtime, 'mode': stat.mode });
        } else if (FS.isFile(stat.mode)) {
          // Performance consideration: storing a normal JavaScript array to a IndexedDB is much slower than storing a typed array.
          // Therefore always convert the file contents to a typed array first before writing the data to IndexedDB.
          node.contents = MEMFS.getFileDataAsTypedArray(node);
          return callback(null, { 'timestamp': stat.mtime, 'mode': stat.mode, 'contents': node.contents });
        } else {
          return callback(new Error('node type not supported'));
        }
      },
  storeLocalEntry:(path, entry, callback) => {
        try {
          if (FS.isDir(entry['mode'])) {
            FS.mkdirTree(path, entry['mode']);
          } else if (FS.isFile(entry['mode'])) {
            FS.writeFile(path, entry['contents'], { canOwn: true });
          } else {
            return callback(new Error('node type not supported'));
          }
  
          FS.chmod(path, entry['mode']);
          FS.utime(path, entry['timestamp'], entry['timestamp']);
        } catch (e) {
          return callback(e);
        }
  
        callback(null);
      },
  removeLocalEntry:(path, callback) => {
        try {
          var stat = FS.stat(path);
  
          if (FS.isDir(stat.mode)) {
            FS.rmdir(path);
          } else if (FS.isFile(stat.mode)) {
            FS.unlink(path);
          }
        } catch (e) {
          return callback(e);
        }
  
        callback(null);
      },
  loadRemoteEntry:(store, path, callback) => {
        var req = store.get(path);
        req.onsuccess = (event) => callback(null, event.target.result);
        req.onerror = (e) => {
          callback(e.target.error);
          e.preventDefault();
        };
      },
  storeRemoteEntry:(store, path, entry, callback) => {
        try {
          var req = store.put(entry, path);
        } catch (e) {
          callback(e);
          return;
        }
        req.onsuccess = (event) => callback();
        req.onerror = (e) => {
          callback(e.target.error);
          e.preventDefault();
        };
      },
  removeRemoteEntry:(store, path, callback) => {
        var req = store.delete(path);
        req.onsuccess = (event) => callback();
        req.onerror = (e) => {
          callback(e.target.error);
          e.preventDefault();
        };
      },
  reconcile:(src, dst, callback) => {
        var total = 0;
  
        var create = [];
        Object.keys(src.entries).forEach((key) => {
          var e = src.entries[key];
          var e2 = dst.entries[key];
          if (!e2 || e['timestamp'].getTime() != e2['timestamp'].getTime()) {
            create.push(key);
            total++;
          }
        });
  
        var remove = [];
        Object.keys(dst.entries).forEach((key) => {
          if (!src.entries[key]) {
            remove.push(key);
            total++;
          }
        });
  
        if (!total) {
          return callback(null);
        }
  
        var errored = false;
        var db = src.type === 'remote' ? src.db : dst.db;
        var transaction = db.transaction([IDBFS.DB_STORE_NAME], 'readwrite');
        var store = transaction.objectStore(IDBFS.DB_STORE_NAME);
  
        function done(err) {
          if (err && !errored) {
            errored = true;
            return callback(err);
          }
        };
  
        // transaction may abort if (for example) there is a QuotaExceededError
        transaction.onerror = transaction.onabort = (e) => {
          done(e.target.error);
          e.preventDefault();
        };
  
        transaction.oncomplete = (e) => {
          if (!errored) {
            callback(null);
          }
        };
  
        // sort paths in ascending order so directory entries are created
        // before the files inside them
        create.sort().forEach((path) => {
          if (dst.type === 'local') {
            IDBFS.loadRemoteEntry(store, path, (err, entry) => {
              if (err) return done(err);
              IDBFS.storeLocalEntry(path, entry, done);
            });
          } else {
            IDBFS.loadLocalEntry(path, (err, entry) => {
              if (err) return done(err);
              IDBFS.storeRemoteEntry(store, path, entry, done);
            });
          }
        });
  
        // sort paths in descending order so files are deleted before their
        // parent directories
        remove.sort().reverse().forEach((path) => {
          if (dst.type === 'local') {
            IDBFS.removeLocalEntry(path, done);
          } else {
            IDBFS.removeRemoteEntry(store, path, done);
          }
        });
      },
  };
  
  
  
  var strError = (errno) => UTF8ToString(_strerror(errno));
  
  var ERRNO_CODES = {
      'EPERM': 63,
      'ENOENT': 44,
      'ESRCH': 71,
      'EINTR': 27,
      'EIO': 29,
      'ENXIO': 60,
      'E2BIG': 1,
      'ENOEXEC': 45,
      'EBADF': 8,
      'ECHILD': 12,
      'EAGAIN': 6,
      'EWOULDBLOCK': 6,
      'ENOMEM': 48,
      'EACCES': 2,
      'EFAULT': 21,
      'ENOTBLK': 105,
      'EBUSY': 10,
      'EEXIST': 20,
      'EXDEV': 75,
      'ENODEV': 43,
      'ENOTDIR': 54,
      'EISDIR': 31,
      'EINVAL': 28,
      'ENFILE': 41,
      'EMFILE': 33,
      'ENOTTY': 59,
      'ETXTBSY': 74,
      'EFBIG': 22,
      'ENOSPC': 51,
      'ESPIPE': 70,
      'EROFS': 69,
      'EMLINK': 34,
      'EPIPE': 64,
      'EDOM': 18,
      'ERANGE': 68,
      'ENOMSG': 49,
      'EIDRM': 24,
      'ECHRNG': 106,
      'EL2NSYNC': 156,
      'EL3HLT': 107,
      'EL3RST': 108,
      'ELNRNG': 109,
      'EUNATCH': 110,
      'ENOCSI': 111,
      'EL2HLT': 112,
      'EDEADLK': 16,
      'ENOLCK': 46,
      'EBADE': 113,
      'EBADR': 114,
      'EXFULL': 115,
      'ENOANO': 104,
      'EBADRQC': 103,
      'EBADSLT': 102,
      'EDEADLOCK': 16,
      'EBFONT': 101,
      'ENOSTR': 100,
      'ENODATA': 116,
      'ETIME': 117,
      'ENOSR': 118,
      'ENONET': 119,
      'ENOPKG': 120,
      'EREMOTE': 121,
      'ENOLINK': 47,
      'EADV': 122,
      'ESRMNT': 123,
      'ECOMM': 124,
      'EPROTO': 65,
      'EMULTIHOP': 36,
      'EDOTDOT': 125,
      'EBADMSG': 9,
      'ENOTUNIQ': 126,
      'EBADFD': 127,
      'EREMCHG': 128,
      'ELIBACC': 129,
      'ELIBBAD': 130,
      'ELIBSCN': 131,
      'ELIBMAX': 132,
      'ELIBEXEC': 133,
      'ENOSYS': 52,
      'ENOTEMPTY': 55,
      'ENAMETOOLONG': 37,
      'ELOOP': 32,
      'EOPNOTSUPP': 138,
      'EPFNOSUPPORT': 139,
      'ECONNRESET': 15,
      'ENOBUFS': 42,
      'EAFNOSUPPORT': 5,
      'EPROTOTYPE': 67,
      'ENOTSOCK': 57,
      'ENOPROTOOPT': 50,
      'ESHUTDOWN': 140,
      'ECONNREFUSED': 14,
      'EADDRINUSE': 3,
      'ECONNABORTED': 13,
      'ENETUNREACH': 40,
      'ENETDOWN': 38,
      'ETIMEDOUT': 73,
      'EHOSTDOWN': 142,
      'EHOSTUNREACH': 23,
      'EINPROGRESS': 26,
      'EALREADY': 7,
      'EDESTADDRREQ': 17,
      'EMSGSIZE': 35,
      'EPROTONOSUPPORT': 66,
      'ESOCKTNOSUPPORT': 137,
      'EADDRNOTAVAIL': 4,
      'ENETRESET': 39,
      'EISCONN': 30,
      'ENOTCONN': 53,
      'ETOOMANYREFS': 141,
      'EUSERS': 136,
      'EDQUOT': 19,
      'ESTALE': 72,
      'ENOTSUP': 138,
      'ENOMEDIUM': 148,
      'EILSEQ': 25,
      'EOVERFLOW': 61,
      'ECANCELED': 11,
      'ENOTRECOVERABLE': 56,
      'EOWNERDEAD': 62,
      'ESTRPIPE': 135,
    };
  var FS = {
  root:null,
  mounts:[],
  devices:{
  },
  streams:[],
  nextInode:1,
  nameTable:null,
  currentPath:"/",
  initialized:false,
  ignorePermissions:true,
  filesystems:null,
  syncFSRequests:0,
  readFiles:{
  },
  ErrnoError:class extends Error {
        name = 'ErrnoError';
        // We set the `name` property to be able to identify `FS.ErrnoError`
        // - the `name` is a standard ECMA-262 property of error objects. Kind of good to have it anyway.
        // - when using PROXYFS, an error can come from an underlying FS
        // as different FS objects have their own FS.ErrnoError each,
        // the test `err instanceof FS.ErrnoError` won't detect an error coming from another filesystem, causing bugs.
        // we'll use the reliable test `err.name == "ErrnoError"` instead
        constructor(errno) {
          super(runtimeInitialized ? strError(errno) : '');
          this.errno = errno;
          for (var key in ERRNO_CODES) {
            if (ERRNO_CODES[key] === errno) {
              this.code = key;
              break;
            }
          }
        }
      },
  FSStream:class {
        shared = {};
        get object() {
          return this.node;
        }
        set object(val) {
          this.node = val;
        }
        get isRead() {
          return (this.flags & 2097155) !== 1;
        }
        get isWrite() {
          return (this.flags & 2097155) !== 0;
        }
        get isAppend() {
          return (this.flags & 1024);
        }
        get flags() {
          return this.shared.flags;
        }
        set flags(val) {
          this.shared.flags = val;
        }
        get position() {
          return this.shared.position;
        }
        set position(val) {
          this.shared.position = val;
        }
      },
  FSNode:class {
        node_ops = {};
        stream_ops = {};
        readMode = 292 | 73;
        writeMode = 146;
        mounted = null;
        constructor(parent, name, mode, rdev) {
          if (!parent) {
            parent = this;  // root node sets parent to itself
          }
          this.parent = parent;
          this.mount = parent.mount;
          this.id = FS.nextInode++;
          this.name = name;
          this.mode = mode;
          this.rdev = rdev;
          this.atime = this.mtime = this.ctime = Date.now();
        }
        get read() {
          return (this.mode & this.readMode) === this.readMode;
        }
        set read(val) {
          val ? this.mode |= this.readMode : this.mode &= ~this.readMode;
        }
        get write() {
          return (this.mode & this.writeMode) === this.writeMode;
        }
        set write(val) {
          val ? this.mode |= this.writeMode : this.mode &= ~this.writeMode;
        }
        get isFolder() {
          return FS.isDir(this.mode);
        }
        get isDevice() {
          return FS.isChrdev(this.mode);
        }
      },
  lookupPath(path, opts = {}) {
        if (!path) {
          throw new FS.ErrnoError(44);
        }
        opts.follow_mount ??= true
  
        if (!PATH.isAbs(path)) {
          path = FS.cwd() + '/' + path;
        }
  
        // limit max consecutive symlinks to 40 (SYMLOOP_MAX).
        linkloop: for (var nlinks = 0; nlinks < 40; nlinks++) {
          // split the absolute path
          var parts = path.split('/').filter((p) => !!p);
  
          // start at the root
          var current = FS.root;
          var current_path = '/';
  
          for (var i = 0; i < parts.length; i++) {
            var islast = (i === parts.length-1);
            if (islast && opts.parent) {
              // stop resolving
              break;
            }
  
            if (parts[i] === '.') {
              continue;
            }
  
            if (parts[i] === '..') {
              current_path = PATH.dirname(current_path);
              if (FS.isRoot(current)) {
                path = current_path + '/' + parts.slice(i + 1).join('/');
                continue linkloop;
              } else {
                current = current.parent;
              }
              continue;
            }
  
            current_path = PATH.join2(current_path, parts[i]);
            try {
              current = FS.lookupNode(current, parts[i]);
            } catch (e) {
              // if noent_okay is true, suppress a ENOENT in the last component
              // and return an object with an undefined node. This is needed for
              // resolving symlinks in the path when creating a file.
              if ((e?.errno === 44) && islast && opts.noent_okay) {
                return { path: current_path };
              }
              throw e;
            }
  
            // jump to the mount's root node if this is a mountpoint
            if (FS.isMountpoint(current) && (!islast || opts.follow_mount)) {
              current = current.mounted.root;
            }
  
            // by default, lookupPath will not follow a symlink if it is the final path component.
            // setting opts.follow = true will override this behavior.
            if (FS.isLink(current.mode) && (!islast || opts.follow)) {
              if (!current.node_ops.readlink) {
                throw new FS.ErrnoError(52);
              }
              var link = current.node_ops.readlink(current);
              if (!PATH.isAbs(link)) {
                link = PATH.dirname(current_path) + '/' + link;
              }
              path = link + '/' + parts.slice(i + 1).join('/');
              continue linkloop;
            }
          }
          return { path: current_path, node: current };
        }
        throw new FS.ErrnoError(32);
      },
  getPath(node) {
        var path;
        while (true) {
          if (FS.isRoot(node)) {
            var mount = node.mount.mountpoint;
            if (!path) return mount;
            return mount[mount.length-1] !== '/' ? `${mount}/${path}` : mount + path;
          }
          path = path ? `${node.name}/${path}` : node.name;
          node = node.parent;
        }
      },
  hashName(parentid, name) {
        var hash = 0;
  
        for (var i = 0; i < name.length; i++) {
          hash = ((hash << 5) - hash + name.charCodeAt(i)) | 0;
        }
        return ((parentid + hash) >>> 0) % FS.nameTable.length;
      },
  hashAddNode(node) {
        var hash = FS.hashName(node.parent.id, node.name);
        node.name_next = FS.nameTable[hash];
        FS.nameTable[hash] = node;
      },
  hashRemoveNode(node) {
        var hash = FS.hashName(node.parent.id, node.name);
        if (FS.nameTable[hash] === node) {
          FS.nameTable[hash] = node.name_next;
        } else {
          var current = FS.nameTable[hash];
          while (current) {
            if (current.name_next === node) {
              current.name_next = node.name_next;
              break;
            }
            current = current.name_next;
          }
        }
      },
  lookupNode(parent, name) {
        var errCode = FS.mayLookup(parent);
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        var hash = FS.hashName(parent.id, name);
        for (var node = FS.nameTable[hash]; node; node = node.name_next) {
          var nodeName = node.name;
          if (node.parent.id === parent.id && nodeName === name) {
            return node;
          }
        }
        // if we failed to find it in the cache, call into the VFS
        return FS.lookup(parent, name);
      },
  createNode(parent, name, mode, rdev) {
        assert(typeof parent == 'object')
        var node = new FS.FSNode(parent, name, mode, rdev);
  
        FS.hashAddNode(node);
  
        return node;
      },
  destroyNode(node) {
        FS.hashRemoveNode(node);
      },
  isRoot(node) {
        return node === node.parent;
      },
  isMountpoint(node) {
        return !!node.mounted;
      },
  isFile(mode) {
        return (mode & 61440) === 32768;
      },
  isDir(mode) {
        return (mode & 61440) === 16384;
      },
  isLink(mode) {
        return (mode & 61440) === 40960;
      },
  isChrdev(mode) {
        return (mode & 61440) === 8192;
      },
  isBlkdev(mode) {
        return (mode & 61440) === 24576;
      },
  isFIFO(mode) {
        return (mode & 61440) === 4096;
      },
  isSocket(mode) {
        return (mode & 49152) === 49152;
      },
  flagsToPermissionString(flag) {
        var perms = ['r', 'w', 'rw'][flag & 3];
        if ((flag & 512)) {
          perms += 'w';
        }
        return perms;
      },
  nodePermissions(node, perms) {
        if (FS.ignorePermissions) {
          return 0;
        }
        // return 0 if any user, group or owner bits are set.
        if (perms.includes('r') && !(node.mode & 292)) {
          return 2;
        } else if (perms.includes('w') && !(node.mode & 146)) {
          return 2;
        } else if (perms.includes('x') && !(node.mode & 73)) {
          return 2;
        }
        return 0;
      },
  mayLookup(dir) {
        if (!FS.isDir(dir.mode)) return 54;
        var errCode = FS.nodePermissions(dir, 'x');
        if (errCode) return errCode;
        if (!dir.node_ops.lookup) return 2;
        return 0;
      },
  mayCreate(dir, name) {
        if (!FS.isDir(dir.mode)) {
          return 54;
        }
        try {
          var node = FS.lookupNode(dir, name);
          return 20;
        } catch (e) {
        }
        return FS.nodePermissions(dir, 'wx');
      },
  mayDelete(dir, name, isdir) {
        var node;
        try {
          node = FS.lookupNode(dir, name);
        } catch (e) {
          return e.errno;
        }
        var errCode = FS.nodePermissions(dir, 'wx');
        if (errCode) {
          return errCode;
        }
        if (isdir) {
          if (!FS.isDir(node.mode)) {
            return 54;
          }
          if (FS.isRoot(node) || FS.getPath(node) === FS.cwd()) {
            return 10;
          }
        } else {
          if (FS.isDir(node.mode)) {
            return 31;
          }
        }
        return 0;
      },
  mayOpen(node, flags) {
        if (!node) {
          return 44;
        }
        if (FS.isLink(node.mode)) {
          return 32;
        } else if (FS.isDir(node.mode)) {
          if (FS.flagsToPermissionString(flags) !== 'r' // opening for write
              || (flags & (512 | 64))) { // TODO: check for O_SEARCH? (== search for dir only)
            return 31;
          }
        }
        return FS.nodePermissions(node, FS.flagsToPermissionString(flags));
      },
  checkOpExists(op, err) {
        if (!op) {
          throw new FS.ErrnoError(err);
        }
        return op;
      },
  MAX_OPEN_FDS:4096,
  nextfd() {
        for (var fd = 0; fd <= FS.MAX_OPEN_FDS; fd++) {
          if (!FS.streams[fd]) {
            return fd;
          }
        }
        throw new FS.ErrnoError(33);
      },
  getStreamChecked(fd) {
        var stream = FS.getStream(fd);
        if (!stream) {
          throw new FS.ErrnoError(8);
        }
        return stream;
      },
  getStream:(fd) => FS.streams[fd],
  createStream(stream, fd = -1) {
        assert(fd >= -1);
  
        // clone it, so we can return an instance of FSStream
        stream = Object.assign(new FS.FSStream(), stream);
        if (fd == -1) {
          fd = FS.nextfd();
        }
        stream.fd = fd;
        FS.streams[fd] = stream;
        return stream;
      },
  closeStream(fd) {
        FS.streams[fd] = null;
      },
  dupStream(origStream, fd = -1) {
        var stream = FS.createStream(origStream, fd);
        stream.stream_ops?.dup?.(stream);
        return stream;
      },
  doSetAttr(stream, node, attr) {
        var setattr = stream?.stream_ops.setattr;
        var arg = setattr ? stream : node;
        setattr ??= node.node_ops.setattr;
        FS.checkOpExists(setattr, 63)
        setattr(arg, attr);
      },
  chrdev_stream_ops:{
  open(stream) {
          var device = FS.getDevice(stream.node.rdev);
          // override node's stream ops with the device's
          stream.stream_ops = device.stream_ops;
          // forward the open call
          stream.stream_ops.open?.(stream);
        },
  llseek() {
          throw new FS.ErrnoError(70);
        },
  },
  major:(dev) => ((dev) >> 8),
  minor:(dev) => ((dev) & 0xff),
  makedev:(ma, mi) => ((ma) << 8 | (mi)),
  registerDevice(dev, ops) {
        FS.devices[dev] = { stream_ops: ops };
      },
  getDevice:(dev) => FS.devices[dev],
  getMounts(mount) {
        var mounts = [];
        var check = [mount];
  
        while (check.length) {
          var m = check.pop();
  
          mounts.push(m);
  
          check.push(...m.mounts);
        }
  
        return mounts;
      },
  syncfs(populate, callback) {
        if (typeof populate == 'function') {
          callback = populate;
          populate = false;
        }
  
        FS.syncFSRequests++;
  
        if (FS.syncFSRequests > 1) {
          err(`warning: ${FS.syncFSRequests} FS.syncfs operations in flight at once, probably just doing extra work`);
        }
  
        var mounts = FS.getMounts(FS.root.mount);
        var completed = 0;
  
        function doCallback(errCode) {
          assert(FS.syncFSRequests > 0);
          FS.syncFSRequests--;
          return callback(errCode);
        }
  
        function done(errCode) {
          if (errCode) {
            if (!done.errored) {
              done.errored = true;
              return doCallback(errCode);
            }
            return;
          }
          if (++completed >= mounts.length) {
            doCallback(null);
          }
        };
  
        // sync all mounts
        mounts.forEach((mount) => {
          if (!mount.type.syncfs) {
            return done(null);
          }
          mount.type.syncfs(mount, populate, done);
        });
      },
  mount(type, opts, mountpoint) {
        if (typeof type == 'string') {
          // The filesystem was not included, and instead we have an error
          // message stored in the variable.
          throw type;
        }
        var root = mountpoint === '/';
        var pseudo = !mountpoint;
        var node;
  
        if (root && FS.root) {
          throw new FS.ErrnoError(10);
        } else if (!root && !pseudo) {
          var lookup = FS.lookupPath(mountpoint, { follow_mount: false });
  
          mountpoint = lookup.path;  // use the absolute path
          node = lookup.node;
  
          if (FS.isMountpoint(node)) {
            throw new FS.ErrnoError(10);
          }
  
          if (!FS.isDir(node.mode)) {
            throw new FS.ErrnoError(54);
          }
        }
  
        var mount = {
          type,
          opts,
          mountpoint,
          mounts: []
        };
  
        // create a root node for the fs
        var mountRoot = type.mount(mount);
        mountRoot.mount = mount;
        mount.root = mountRoot;
  
        if (root) {
          FS.root = mountRoot;
        } else if (node) {
          // set as a mountpoint
          node.mounted = mount;
  
          // add the new mount to the current mount's children
          if (node.mount) {
            node.mount.mounts.push(mount);
          }
        }
  
        return mountRoot;
      },
  unmount(mountpoint) {
        var lookup = FS.lookupPath(mountpoint, { follow_mount: false });
  
        if (!FS.isMountpoint(lookup.node)) {
          throw new FS.ErrnoError(28);
        }
  
        // destroy the nodes for this mount, and all its child mounts
        var node = lookup.node;
        var mount = node.mounted;
        var mounts = FS.getMounts(mount);
  
        Object.keys(FS.nameTable).forEach((hash) => {
          var current = FS.nameTable[hash];
  
          while (current) {
            var next = current.name_next;
  
            if (mounts.includes(current.mount)) {
              FS.destroyNode(current);
            }
  
            current = next;
          }
        });
  
        // no longer a mountpoint
        node.mounted = null;
  
        // remove this mount from the child mounts
        var idx = node.mount.mounts.indexOf(mount);
        assert(idx !== -1);
        node.mount.mounts.splice(idx, 1);
      },
  lookup(parent, name) {
        return parent.node_ops.lookup(parent, name);
      },
  mknod(path, mode, dev) {
        var lookup = FS.lookupPath(path, { parent: true });
        var parent = lookup.node;
        var name = PATH.basename(path);
        if (!name) {
          throw new FS.ErrnoError(28);
        }
        if (name === '.' || name === '..') {
          throw new FS.ErrnoError(20);
        }
        var errCode = FS.mayCreate(parent, name);
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        if (!parent.node_ops.mknod) {
          throw new FS.ErrnoError(63);
        }
        return parent.node_ops.mknod(parent, name, mode, dev);
      },
  statfs(path) {
        return FS.statfsNode(FS.lookupPath(path, {follow: true}).node);
      },
  statfsStream(stream) {
        // We keep a separate statfsStream function because noderawfs overrides
        // it. In noderawfs, stream.node is sometimes null. Instead, we need to
        // look at stream.path.
        return FS.statfsNode(stream.node);
      },
  statfsNode(node) {
        // NOTE: None of the defaults here are true. We're just returning safe and
        //       sane values. Currently nodefs and rawfs replace these defaults,
        //       other file systems leave them alone.
        var rtn = {
          bsize: 4096,
          frsize: 4096,
          blocks: 1e6,
          bfree: 5e5,
          bavail: 5e5,
          files: FS.nextInode,
          ffree: FS.nextInode - 1,
          fsid: 42,
          flags: 2,
          namelen: 255,
        };
  
        if (node.node_ops.statfs) {
          Object.assign(rtn, node.node_ops.statfs(node.mount.opts.root));
        }
        return rtn;
      },
  create(path, mode = 0o666) {
        mode &= 4095;
        mode |= 32768;
        return FS.mknod(path, mode, 0);
      },
  mkdir(path, mode = 0o777) {
        mode &= 511 | 512;
        mode |= 16384;
        return FS.mknod(path, mode, 0);
      },
  mkdirTree(path, mode) {
        var dirs = path.split('/');
        var d = '';
        for (var dir of dirs) {
          if (!dir) continue;
          if (d || PATH.isAbs(path)) d += '/';
          d += dir;
          try {
            FS.mkdir(d, mode);
          } catch(e) {
            if (e.errno != 20) throw e;
          }
        }
      },
  mkdev(path, mode, dev) {
        if (typeof dev == 'undefined') {
          dev = mode;
          mode = 0o666;
        }
        mode |= 8192;
        return FS.mknod(path, mode, dev);
      },
  symlink(oldpath, newpath) {
        if (!PATH_FS.resolve(oldpath)) {
          throw new FS.ErrnoError(44);
        }
        var lookup = FS.lookupPath(newpath, { parent: true });
        var parent = lookup.node;
        if (!parent) {
          throw new FS.ErrnoError(44);
        }
        var newname = PATH.basename(newpath);
        var errCode = FS.mayCreate(parent, newname);
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        if (!parent.node_ops.symlink) {
          throw new FS.ErrnoError(63);
        }
        return parent.node_ops.symlink(parent, newname, oldpath);
      },
  rename(old_path, new_path) {
        var old_dirname = PATH.dirname(old_path);
        var new_dirname = PATH.dirname(new_path);
        var old_name = PATH.basename(old_path);
        var new_name = PATH.basename(new_path);
        // parents must exist
        var lookup, old_dir, new_dir;
  
        // let the errors from non existent directories percolate up
        lookup = FS.lookupPath(old_path, { parent: true });
        old_dir = lookup.node;
        lookup = FS.lookupPath(new_path, { parent: true });
        new_dir = lookup.node;
  
        if (!old_dir || !new_dir) throw new FS.ErrnoError(44);
        // need to be part of the same mount
        if (old_dir.mount !== new_dir.mount) {
          throw new FS.ErrnoError(75);
        }
        // source must exist
        var old_node = FS.lookupNode(old_dir, old_name);
        // old path should not be an ancestor of the new path
        var relative = PATH_FS.relative(old_path, new_dirname);
        if (relative.charAt(0) !== '.') {
          throw new FS.ErrnoError(28);
        }
        // new path should not be an ancestor of the old path
        relative = PATH_FS.relative(new_path, old_dirname);
        if (relative.charAt(0) !== '.') {
          throw new FS.ErrnoError(55);
        }
        // see if the new path already exists
        var new_node;
        try {
          new_node = FS.lookupNode(new_dir, new_name);
        } catch (e) {
          // not fatal
        }
        // early out if nothing needs to change
        if (old_node === new_node) {
          return;
        }
        // we'll need to delete the old entry
        var isdir = FS.isDir(old_node.mode);
        var errCode = FS.mayDelete(old_dir, old_name, isdir);
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        // need delete permissions if we'll be overwriting.
        // need create permissions if new doesn't already exist.
        errCode = new_node ?
          FS.mayDelete(new_dir, new_name, isdir) :
          FS.mayCreate(new_dir, new_name);
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        if (!old_dir.node_ops.rename) {
          throw new FS.ErrnoError(63);
        }
        if (FS.isMountpoint(old_node) || (new_node && FS.isMountpoint(new_node))) {
          throw new FS.ErrnoError(10);
        }
        // if we are going to change the parent, check write permissions
        if (new_dir !== old_dir) {
          errCode = FS.nodePermissions(old_dir, 'w');
          if (errCode) {
            throw new FS.ErrnoError(errCode);
          }
        }
        // remove the node from the lookup hash
        FS.hashRemoveNode(old_node);
        // do the underlying fs rename
        try {
          old_dir.node_ops.rename(old_node, new_dir, new_name);
          // update old node (we do this here to avoid each backend
          // needing to)
          old_node.parent = new_dir;
        } catch (e) {
          throw e;
        } finally {
          // add the node back to the hash (in case node_ops.rename
          // changed its name)
          FS.hashAddNode(old_node);
        }
      },
  rmdir(path) {
        var lookup = FS.lookupPath(path, { parent: true });
        var parent = lookup.node;
        var name = PATH.basename(path);
        var node = FS.lookupNode(parent, name);
        var errCode = FS.mayDelete(parent, name, true);
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        if (!parent.node_ops.rmdir) {
          throw new FS.ErrnoError(63);
        }
        if (FS.isMountpoint(node)) {
          throw new FS.ErrnoError(10);
        }
        parent.node_ops.rmdir(parent, name);
        FS.destroyNode(node);
      },
  readdir(path) {
        var lookup = FS.lookupPath(path, { follow: true });
        var node = lookup.node;
        var readdir = FS.checkOpExists(node.node_ops.readdir, 54);
        return readdir(node);
      },
  unlink(path) {
        var lookup = FS.lookupPath(path, { parent: true });
        var parent = lookup.node;
        if (!parent) {
          throw new FS.ErrnoError(44);
        }
        var name = PATH.basename(path);
        var node = FS.lookupNode(parent, name);
        var errCode = FS.mayDelete(parent, name, false);
        if (errCode) {
          // According to POSIX, we should map EISDIR to EPERM, but
          // we instead do what Linux does (and we must, as we use
          // the musl linux libc).
          throw new FS.ErrnoError(errCode);
        }
        if (!parent.node_ops.unlink) {
          throw new FS.ErrnoError(63);
        }
        if (FS.isMountpoint(node)) {
          throw new FS.ErrnoError(10);
        }
        parent.node_ops.unlink(parent, name);
        FS.destroyNode(node);
      },
  readlink(path) {
        var lookup = FS.lookupPath(path);
        var link = lookup.node;
        if (!link) {
          throw new FS.ErrnoError(44);
        }
        if (!link.node_ops.readlink) {
          throw new FS.ErrnoError(28);
        }
        return link.node_ops.readlink(link);
      },
  stat(path, dontFollow) {
        var lookup = FS.lookupPath(path, { follow: !dontFollow });
        var node = lookup.node;
        var getattr = FS.checkOpExists(node.node_ops.getattr, 63);
        return getattr(node);
      },
  fstat(fd) {
        var stream = FS.getStreamChecked(fd);
        var node = stream.node;
        var getattr = stream.stream_ops.getattr;
        var arg = getattr ? stream : node;
        getattr ??= node.node_ops.getattr;
        FS.checkOpExists(getattr, 63)
        return getattr(arg);
      },
  lstat(path) {
        return FS.stat(path, true);
      },
  doChmod(stream, node, mode, dontFollow) {
        FS.doSetAttr(stream, node, {
          mode: (mode & 4095) | (node.mode & ~4095),
          ctime: Date.now(),
          dontFollow
        });
      },
  chmod(path, mode, dontFollow) {
        var node;
        if (typeof path == 'string') {
          var lookup = FS.lookupPath(path, { follow: !dontFollow });
          node = lookup.node;
        } else {
          node = path;
        }
        FS.doChmod(null, node, mode, dontFollow);
      },
  lchmod(path, mode) {
        FS.chmod(path, mode, true);
      },
  fchmod(fd, mode) {
        var stream = FS.getStreamChecked(fd);
        FS.doChmod(stream, stream.node, mode, false);
      },
  doChown(stream, node, dontFollow) {
        FS.doSetAttr(stream, node, {
          timestamp: Date.now(),
          dontFollow
          // we ignore the uid / gid for now
        });
      },
  chown(path, uid, gid, dontFollow) {
        var node;
        if (typeof path == 'string') {
          var lookup = FS.lookupPath(path, { follow: !dontFollow });
          node = lookup.node;
        } else {
          node = path;
        }
        FS.doChown(null, node, dontFollow);
      },
  lchown(path, uid, gid) {
        FS.chown(path, uid, gid, true);
      },
  fchown(fd, uid, gid) {
        var stream = FS.getStreamChecked(fd);
        FS.doChown(stream, stream.node, false);
      },
  doTruncate(stream, node, len) {
        if (FS.isDir(node.mode)) {
          throw new FS.ErrnoError(31);
        }
        if (!FS.isFile(node.mode)) {
          throw new FS.ErrnoError(28);
        }
        var errCode = FS.nodePermissions(node, 'w');
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        FS.doSetAttr(stream, node, {
          size: len,
          timestamp: Date.now()
        });
      },
  truncate(path, len) {
        if (len < 0) {
          throw new FS.ErrnoError(28);
        }
        var node;
        if (typeof path == 'string') {
          var lookup = FS.lookupPath(path, { follow: true });
          node = lookup.node;
        } else {
          node = path;
        }
        FS.doTruncate(null, node, len);
      },
  ftruncate(fd, len) {
        var stream = FS.getStreamChecked(fd);
        if (len < 0 || (stream.flags & 2097155) === 0) {
          throw new FS.ErrnoError(28);
        }
        FS.doTruncate(stream, stream.node, len);
      },
  utime(path, atime, mtime) {
        var lookup = FS.lookupPath(path, { follow: true });
        var node = lookup.node;
        var setattr = FS.checkOpExists(node.node_ops.setattr, 63);
        setattr(node, {
          atime: atime,
          mtime: mtime
        });
      },
  open(path, flags, mode = 0o666) {
        if (path === "") {
          throw new FS.ErrnoError(44);
        }
        flags = typeof flags == 'string' ? FS_modeStringToFlags(flags) : flags;
        if ((flags & 64)) {
          mode = (mode & 4095) | 32768;
        } else {
          mode = 0;
        }
        var node;
        var isDirPath;
        if (typeof path == 'object') {
          node = path;
        } else {
          isDirPath = path.endsWith("/");
          // noent_okay makes it so that if the final component of the path
          // doesn't exist, lookupPath returns `node: undefined`. `path` will be
          // updated to point to the target of all symlinks.
          var lookup = FS.lookupPath(path, {
            follow: !(flags & 131072),
            noent_okay: true
          });
          node = lookup.node;
          path = lookup.path;
        }
        // perhaps we need to create the node
        var created = false;
        if ((flags & 64)) {
          if (node) {
            // if O_CREAT and O_EXCL are set, error out if the node already exists
            if ((flags & 128)) {
              throw new FS.ErrnoError(20);
            }
          } else if (isDirPath) {
            throw new FS.ErrnoError(31);
          } else {
            // node doesn't exist, try to create it
            // Ignore the permission bits here to ensure we can `open` this new
            // file below. We use chmod below the apply the permissions once the
            // file is open.
            node = FS.mknod(path, mode | 0o777, 0);
            created = true;
          }
        }
        if (!node) {
          throw new FS.ErrnoError(44);
        }
        // can't truncate a device
        if (FS.isChrdev(node.mode)) {
          flags &= ~512;
        }
        // if asked only for a directory, then this must be one
        if ((flags & 65536) && !FS.isDir(node.mode)) {
          throw new FS.ErrnoError(54);
        }
        // check permissions, if this is not a file we just created now (it is ok to
        // create and write to a file with read-only permissions; it is read-only
        // for later use)
        if (!created) {
          var errCode = FS.mayOpen(node, flags);
          if (errCode) {
            throw new FS.ErrnoError(errCode);
          }
        }
        // do truncation if necessary
        if ((flags & 512) && !created) {
          FS.truncate(node, 0);
        }
        // we've already handled these, don't pass down to the underlying vfs
        flags &= ~(128 | 512 | 131072);
  
        // register the stream with the filesystem
        var stream = FS.createStream({
          node,
          path: FS.getPath(node),  // we want the absolute path to the node
          flags,
          seekable: true,
          position: 0,
          stream_ops: node.stream_ops,
          // used by the file family libc calls (fopen, fwrite, ferror, etc.)
          ungotten: [],
          error: false
        });
        // call the new stream's open function
        if (stream.stream_ops.open) {
          stream.stream_ops.open(stream);
        }
        if (created) {
          FS.chmod(node, mode & 0o777);
        }
        if (Module['logReadFiles'] && !(flags & 1)) {
          if (!(path in FS.readFiles)) {
            FS.readFiles[path] = 1;
          }
        }
        return stream;
      },
  close(stream) {
        if (FS.isClosed(stream)) {
          throw new FS.ErrnoError(8);
        }
        if (stream.getdents) stream.getdents = null; // free readdir state
        try {
          if (stream.stream_ops.close) {
            stream.stream_ops.close(stream);
          }
        } catch (e) {
          throw e;
        } finally {
          FS.closeStream(stream.fd);
        }
        stream.fd = null;
      },
  isClosed(stream) {
        return stream.fd === null;
      },
  llseek(stream, offset, whence) {
        if (FS.isClosed(stream)) {
          throw new FS.ErrnoError(8);
        }
        if (!stream.seekable || !stream.stream_ops.llseek) {
          throw new FS.ErrnoError(70);
        }
        if (whence != 0 && whence != 1 && whence != 2) {
          throw new FS.ErrnoError(28);
        }
        stream.position = stream.stream_ops.llseek(stream, offset, whence);
        stream.ungotten = [];
        return stream.position;
      },
  read(stream, buffer, offset, length, position) {
        assert(offset >= 0);
        if (length < 0 || position < 0) {
          throw new FS.ErrnoError(28);
        }
        if (FS.isClosed(stream)) {
          throw new FS.ErrnoError(8);
        }
        if ((stream.flags & 2097155) === 1) {
          throw new FS.ErrnoError(8);
        }
        if (FS.isDir(stream.node.mode)) {
          throw new FS.ErrnoError(31);
        }
        if (!stream.stream_ops.read) {
          throw new FS.ErrnoError(28);
        }
        var seeking = typeof position != 'undefined';
        if (!seeking) {
          position = stream.position;
        } else if (!stream.seekable) {
          throw new FS.ErrnoError(70);
        }
        var bytesRead = stream.stream_ops.read(stream, buffer, offset, length, position);
        if (!seeking) stream.position += bytesRead;
        return bytesRead;
      },
  write(stream, buffer, offset, length, position, canOwn) {
        assert(offset >= 0);
        if (length < 0 || position < 0) {
          throw new FS.ErrnoError(28);
        }
        if (FS.isClosed(stream)) {
          throw new FS.ErrnoError(8);
        }
        if ((stream.flags & 2097155) === 0) {
          throw new FS.ErrnoError(8);
        }
        if (FS.isDir(stream.node.mode)) {
          throw new FS.ErrnoError(31);
        }
        if (!stream.stream_ops.write) {
          throw new FS.ErrnoError(28);
        }
        if (stream.seekable && stream.flags & 1024) {
          // seek to the end before writing in append mode
          FS.llseek(stream, 0, 2);
        }
        var seeking = typeof position != 'undefined';
        if (!seeking) {
          position = stream.position;
        } else if (!stream.seekable) {
          throw new FS.ErrnoError(70);
        }
        var bytesWritten = stream.stream_ops.write(stream, buffer, offset, length, position, canOwn);
        if (!seeking) stream.position += bytesWritten;
        return bytesWritten;
      },
  mmap(stream, length, position, prot, flags) {
        // User requests writing to file (prot & PROT_WRITE != 0).
        // Checking if we have permissions to write to the file unless
        // MAP_PRIVATE flag is set. According to POSIX spec it is possible
        // to write to file opened in read-only mode with MAP_PRIVATE flag,
        // as all modifications will be visible only in the memory of
        // the current process.
        if ((prot & 2) !== 0
            && (flags & 2) === 0
            && (stream.flags & 2097155) !== 2) {
          throw new FS.ErrnoError(2);
        }
        if ((stream.flags & 2097155) === 1) {
          throw new FS.ErrnoError(2);
        }
        if (!stream.stream_ops.mmap) {
          throw new FS.ErrnoError(43);
        }
        if (!length) {
          throw new FS.ErrnoError(28);
        }
        return stream.stream_ops.mmap(stream, length, position, prot, flags);
      },
  msync(stream, buffer, offset, length, mmapFlags) {
        assert(offset >= 0);
        if (!stream.stream_ops.msync) {
          return 0;
        }
        return stream.stream_ops.msync(stream, buffer, offset, length, mmapFlags);
      },
  ioctl(stream, cmd, arg) {
        if (!stream.stream_ops.ioctl) {
          throw new FS.ErrnoError(59);
        }
        return stream.stream_ops.ioctl(stream, cmd, arg);
      },
  readFile(path, opts = {}) {
        opts.flags = opts.flags || 0;
        opts.encoding = opts.encoding || 'binary';
        if (opts.encoding !== 'utf8' && opts.encoding !== 'binary') {
          throw new Error(`Invalid encoding type "${opts.encoding}"`);
        }
        var ret;
        var stream = FS.open(path, opts.flags);
        var stat = FS.stat(path);
        var length = stat.size;
        var buf = new Uint8Array(length);
        FS.read(stream, buf, 0, length, 0);
        if (opts.encoding === 'utf8') {
          ret = UTF8ArrayToString(buf);
        } else if (opts.encoding === 'binary') {
          ret = buf;
        }
        FS.close(stream);
        return ret;
      },
  writeFile(path, data, opts = {}) {
        opts.flags = opts.flags || 577;
        var stream = FS.open(path, opts.flags, opts.mode);
        if (typeof data == 'string') {
          var buf = new Uint8Array(lengthBytesUTF8(data)+1);
          var actualNumBytes = stringToUTF8Array(data, buf, 0, buf.length);
          FS.write(stream, buf, 0, actualNumBytes, undefined, opts.canOwn);
        } else if (ArrayBuffer.isView(data)) {
          FS.write(stream, data, 0, data.byteLength, undefined, opts.canOwn);
        } else {
          throw new Error('Unsupported data type');
        }
        FS.close(stream);
      },
  cwd:() => FS.currentPath,
  chdir(path) {
        var lookup = FS.lookupPath(path, { follow: true });
        if (lookup.node === null) {
          throw new FS.ErrnoError(44);
        }
        if (!FS.isDir(lookup.node.mode)) {
          throw new FS.ErrnoError(54);
        }
        var errCode = FS.nodePermissions(lookup.node, 'x');
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        FS.currentPath = lookup.path;
      },
  createDefaultDirectories() {
        FS.mkdir('/tmp');
        FS.mkdir('/home');
        FS.mkdir('/home/web_user');
      },
  createDefaultDevices() {
        // create /dev
        FS.mkdir('/dev');
        // setup /dev/null
        FS.registerDevice(FS.makedev(1, 3), {
          read: () => 0,
          write: (stream, buffer, offset, length, pos) => length,
          llseek: () => 0,
        });
        FS.mkdev('/dev/null', FS.makedev(1, 3));
        // setup /dev/tty and /dev/tty1
        // stderr needs to print output using err() rather than out()
        // so we register a second tty just for it.
        TTY.register(FS.makedev(5, 0), TTY.default_tty_ops);
        TTY.register(FS.makedev(6, 0), TTY.default_tty1_ops);
        FS.mkdev('/dev/tty', FS.makedev(5, 0));
        FS.mkdev('/dev/tty1', FS.makedev(6, 0));
        // setup /dev/[u]random
        // use a buffer to avoid overhead of individual crypto calls per byte
        var randomBuffer = new Uint8Array(1024), randomLeft = 0;
        var randomByte = () => {
          if (randomLeft === 0) {
            randomFill(randomBuffer);
            randomLeft = randomBuffer.byteLength;
          }
          return randomBuffer[--randomLeft];
        };
        FS.createDevice('/dev', 'random', randomByte);
        FS.createDevice('/dev', 'urandom', randomByte);
        // we're not going to emulate the actual shm device,
        // just create the tmp dirs that reside in it commonly
        FS.mkdir('/dev/shm');
        FS.mkdir('/dev/shm/tmp');
      },
  createSpecialDirectories() {
        // create /proc/self/fd which allows /proc/self/fd/6 => readlink gives the
        // name of the stream for fd 6 (see test_unistd_ttyname)
        FS.mkdir('/proc');
        var proc_self = FS.mkdir('/proc/self');
        FS.mkdir('/proc/self/fd');
        FS.mount({
          mount() {
            var node = FS.createNode(proc_self, 'fd', 16895, 73);
            node.stream_ops = {
              llseek: MEMFS.stream_ops.llseek,
            };
            node.node_ops = {
              lookup(parent, name) {
                var fd = +name;
                var stream = FS.getStreamChecked(fd);
                var ret = {
                  parent: null,
                  mount: { mountpoint: 'fake' },
                  node_ops: { readlink: () => stream.path },
                  id: fd + 1,
                };
                ret.parent = ret; // make it look like a simple root node
                return ret;
              },
              readdir() {
                return Array.from(FS.streams.entries())
                  .filter(([k, v]) => v)
                  .map(([k, v]) => k.toString());
              }
            };
            return node;
          }
        }, {}, '/proc/self/fd');
      },
  createStandardStreams(input, output, error) {
        // TODO deprecate the old functionality of a single
        // input / output callback and that utilizes FS.createDevice
        // and instead require a unique set of stream ops
  
        // by default, we symlink the standard streams to the
        // default tty devices. however, if the standard streams
        // have been overwritten we create a unique device for
        // them instead.
        if (input) {
          FS.createDevice('/dev', 'stdin', input);
        } else {
          FS.symlink('/dev/tty', '/dev/stdin');
        }
        if (output) {
          FS.createDevice('/dev', 'stdout', null, output);
        } else {
          FS.symlink('/dev/tty', '/dev/stdout');
        }
        if (error) {
          FS.createDevice('/dev', 'stderr', null, error);
        } else {
          FS.symlink('/dev/tty1', '/dev/stderr');
        }
  
        // open default streams for the stdin, stdout and stderr devices
        var stdin = FS.open('/dev/stdin', 0);
        var stdout = FS.open('/dev/stdout', 1);
        var stderr = FS.open('/dev/stderr', 1);
        assert(stdin.fd === 0, `invalid handle for stdin (${stdin.fd})`);
        assert(stdout.fd === 1, `invalid handle for stdout (${stdout.fd})`);
        assert(stderr.fd === 2, `invalid handle for stderr (${stderr.fd})`);
      },
  staticInit() {
        FS.nameTable = new Array(4096);
  
        FS.mount(MEMFS, {}, '/');
  
        FS.createDefaultDirectories();
        FS.createDefaultDevices();
        FS.createSpecialDirectories();
  
        FS.filesystems = {
          'MEMFS': MEMFS,
          'IDBFS': IDBFS,
        };
      },
  init(input, output, error) {
        assert(!FS.initialized, 'FS.init was previously called. If you want to initialize later with custom parameters, remove any earlier calls (note that one is automatically added to the generated code)');
        FS.initialized = true;
  
        // Allow Module.stdin etc. to provide defaults, if none explicitly passed to us here
        input ??= Module['stdin'];
        output ??= Module['stdout'];
        error ??= Module['stderr'];
  
        FS.createStandardStreams(input, output, error);
      },
  quit() {
        FS.initialized = false;
        // force-flush all streams, so we get musl std streams printed out
        _fflush(0);
        // close all of our streams
        for (var stream of FS.streams) {
          if (stream) {
            FS.close(stream);
          }
        }
      },
  findObject(path, dontResolveLastLink) {
        var ret = FS.analyzePath(path, dontResolveLastLink);
        if (!ret.exists) {
          return null;
        }
        return ret.object;
      },
  analyzePath(path, dontResolveLastLink) {
        // operate from within the context of the symlink's target
        try {
          var lookup = FS.lookupPath(path, { follow: !dontResolveLastLink });
          path = lookup.path;
        } catch (e) {
        }
        var ret = {
          isRoot: false, exists: false, error: 0, name: null, path: null, object: null,
          parentExists: false, parentPath: null, parentObject: null
        };
        try {
          var lookup = FS.lookupPath(path, { parent: true });
          ret.parentExists = true;
          ret.parentPath = lookup.path;
          ret.parentObject = lookup.node;
          ret.name = PATH.basename(path);
          lookup = FS.lookupPath(path, { follow: !dontResolveLastLink });
          ret.exists = true;
          ret.path = lookup.path;
          ret.object = lookup.node;
          ret.name = lookup.node.name;
          ret.isRoot = lookup.path === '/';
        } catch (e) {
          ret.error = e.errno;
        };
        return ret;
      },
  createPath(parent, path, canRead, canWrite) {
        parent = typeof parent == 'string' ? parent : FS.getPath(parent);
        var parts = path.split('/').reverse();
        while (parts.length) {
          var part = parts.pop();
          if (!part) continue;
          var current = PATH.join2(parent, part);
          try {
            FS.mkdir(current);
          } catch (e) {
            if (e.errno != 20) throw e;
          }
          parent = current;
        }
        return current;
      },
  createFile(parent, name, properties, canRead, canWrite) {
        var path = PATH.join2(typeof parent == 'string' ? parent : FS.getPath(parent), name);
        var mode = FS_getMode(canRead, canWrite);
        return FS.create(path, mode);
      },
  createDataFile(parent, name, data, canRead, canWrite, canOwn) {
        var path = name;
        if (parent) {
          parent = typeof parent == 'string' ? parent : FS.getPath(parent);
          path = name ? PATH.join2(parent, name) : parent;
        }
        var mode = FS_getMode(canRead, canWrite);
        var node = FS.create(path, mode);
        if (data) {
          if (typeof data == 'string') {
            var arr = new Array(data.length);
            for (var i = 0, len = data.length; i < len; ++i) arr[i] = data.charCodeAt(i);
            data = arr;
          }
          // make sure we can write to the file
          FS.chmod(node, mode | 146);
          var stream = FS.open(node, 577);
          FS.write(stream, data, 0, data.length, 0, canOwn);
          FS.close(stream);
          FS.chmod(node, mode);
        }
      },
  createDevice(parent, name, input, output) {
        var path = PATH.join2(typeof parent == 'string' ? parent : FS.getPath(parent), name);
        var mode = FS_getMode(!!input, !!output);
        FS.createDevice.major ??= 64;
        var dev = FS.makedev(FS.createDevice.major++, 0);
        // Create a fake device that a set of stream ops to emulate
        // the old behavior.
        FS.registerDevice(dev, {
          open(stream) {
            stream.seekable = false;
          },
          close(stream) {
            // flush any pending line data
            if (output?.buffer?.length) {
              output(10);
            }
          },
          read(stream, buffer, offset, length, pos /* ignored */) {
            var bytesRead = 0;
            for (var i = 0; i < length; i++) {
              var result;
              try {
                result = input();
              } catch (e) {
                throw new FS.ErrnoError(29);
              }
              if (result === undefined && bytesRead === 0) {
                throw new FS.ErrnoError(6);
              }
              if (result === null || result === undefined) break;
              bytesRead++;
              buffer[offset+i] = result;
            }
            if (bytesRead) {
              stream.node.atime = Date.now();
            }
            return bytesRead;
          },
          write(stream, buffer, offset, length, pos) {
            for (var i = 0; i < length; i++) {
              try {
                output(buffer[offset+i]);
              } catch (e) {
                throw new FS.ErrnoError(29);
              }
            }
            if (length) {
              stream.node.mtime = stream.node.ctime = Date.now();
            }
            return i;
          }
        });
        return FS.mkdev(path, mode, dev);
      },
  forceLoadFile(obj) {
        if (obj.isDevice || obj.isFolder || obj.link || obj.contents) return true;
        if (typeof XMLHttpRequest != 'undefined') {
          throw new Error("Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread.");
        } else { // Command-line.
          try {
            obj.contents = readBinary(obj.url);
            obj.usedBytes = obj.contents.length;
          } catch (e) {
            throw new FS.ErrnoError(29);
          }
        }
      },
  createLazyFile(parent, name, url, canRead, canWrite) {
        // Lazy chunked Uint8Array (implements get and length from Uint8Array).
        // Actual getting is abstracted away for eventual reuse.
        class LazyUint8Array {
          lengthKnown = false;
          chunks = []; // Loaded chunks. Index is the chunk number
          get(idx) {
            if (idx > this.length-1 || idx < 0) {
              return undefined;
            }
            var chunkOffset = idx % this.chunkSize;
            var chunkNum = (idx / this.chunkSize)|0;
            return this.getter(chunkNum)[chunkOffset];
          }
          setDataGetter(getter) {
            this.getter = getter;
          }
          cacheLength() {
            // Find length
            var xhr = new XMLHttpRequest();
            xhr.open('HEAD', url, false);
            xhr.send(null);
            if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
            var datalength = Number(xhr.getResponseHeader("Content-length"));
            var header;
            var hasByteServing = (header = xhr.getResponseHeader("Accept-Ranges")) && header === "bytes";
            var usesGzip = (header = xhr.getResponseHeader("Content-Encoding")) && header === "gzip";
  
            var chunkSize = 1024*1024; // Chunk size in bytes
  
            if (!hasByteServing) chunkSize = datalength;
  
            // Function to get a range from the remote URL.
            var doXHR = (from, to) => {
              if (from > to) throw new Error("invalid range (" + from + ", " + to + ") or no bytes requested!");
              if (to > datalength-1) throw new Error("only " + datalength + " bytes available! programmer error!");
  
              // TODO: Use mozResponseArrayBuffer, responseStream, etc. if available.
              var xhr = new XMLHttpRequest();
              xhr.open('GET', url, false);
              if (datalength !== chunkSize) xhr.setRequestHeader("Range", "bytes=" + from + "-" + to);
  
              // Some hints to the browser that we want binary data.
              xhr.responseType = 'arraybuffer';
              if (xhr.overrideMimeType) {
                xhr.overrideMimeType('text/plain; charset=x-user-defined');
              }
  
              xhr.send(null);
              if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
              if (xhr.response !== undefined) {
                return new Uint8Array(/** @type{Array<number>} */(xhr.response || []));
              }
              return intArrayFromString(xhr.responseText || '', true);
            };
            var lazyArray = this;
            lazyArray.setDataGetter((chunkNum) => {
              var start = chunkNum * chunkSize;
              var end = (chunkNum+1) * chunkSize - 1; // including this byte
              end = Math.min(end, datalength-1); // if datalength-1 is selected, this is the last block
              if (typeof lazyArray.chunks[chunkNum] == 'undefined') {
                lazyArray.chunks[chunkNum] = doXHR(start, end);
              }
              if (typeof lazyArray.chunks[chunkNum] == 'undefined') throw new Error('doXHR failed!');
              return lazyArray.chunks[chunkNum];
            });
  
            if (usesGzip || !datalength) {
              // if the server uses gzip or doesn't supply the length, we have to download the whole file to get the (uncompressed) length
              chunkSize = datalength = 1; // this will force getter(0)/doXHR do download the whole file
              datalength = this.getter(0).length;
              chunkSize = datalength;
              out("LazyFiles on gzip forces download of the whole file when length is accessed");
            }
  
            this._length = datalength;
            this._chunkSize = chunkSize;
            this.lengthKnown = true;
          }
          get length() {
            if (!this.lengthKnown) {
              this.cacheLength();
            }
            return this._length;
          }
          get chunkSize() {
            if (!this.lengthKnown) {
              this.cacheLength();
            }
            return this._chunkSize;
          }
        }
  
        if (typeof XMLHttpRequest != 'undefined') {
          if (!ENVIRONMENT_IS_WORKER) throw 'Cannot do synchronous binary XHRs outside webworkers in modern browsers. Use --embed-file or --preload-file in emcc';
          var lazyArray = new LazyUint8Array();
          var properties = { isDevice: false, contents: lazyArray };
        } else {
          var properties = { isDevice: false, url: url };
        }
  
        var node = FS.createFile(parent, name, properties, canRead, canWrite);
        // This is a total hack, but I want to get this lazy file code out of the
        // core of MEMFS. If we want to keep this lazy file concept I feel it should
        // be its own thin LAZYFS proxying calls to MEMFS.
        if (properties.contents) {
          node.contents = properties.contents;
        } else if (properties.url) {
          node.contents = null;
          node.url = properties.url;
        }
        // Add a function that defers querying the file size until it is asked the first time.
        Object.defineProperties(node, {
          usedBytes: {
            get: function() { return this.contents.length; }
          }
        });
        // override each stream op with one that tries to force load the lazy file first
        var stream_ops = {};
        var keys = Object.keys(node.stream_ops);
        keys.forEach((key) => {
          var fn = node.stream_ops[key];
          stream_ops[key] = (...args) => {
            FS.forceLoadFile(node);
            return fn(...args);
          };
        });
        function writeChunks(stream, buffer, offset, length, position) {
          var contents = stream.node.contents;
          if (position >= contents.length)
            return 0;
          var size = Math.min(contents.length - position, length);
          assert(size >= 0);
          if (contents.slice) { // normal array
            for (var i = 0; i < size; i++) {
              buffer[offset + i] = contents[position + i];
            }
          } else {
            for (var i = 0; i < size; i++) { // LazyUint8Array from sync binary XHR
              buffer[offset + i] = contents.get(position + i);
            }
          }
          return size;
        }
        // use a custom read function
        stream_ops.read = (stream, buffer, offset, length, position) => {
          FS.forceLoadFile(node);
          return writeChunks(stream, buffer, offset, length, position)
        };
        // use a custom mmap function
        stream_ops.mmap = (stream, length, position, prot, flags) => {
          FS.forceLoadFile(node);
          var ptr = mmapAlloc(length);
          if (!ptr) {
            throw new FS.ErrnoError(48);
          }
          writeChunks(stream, HEAP8, ptr, length, position);
          return { ptr, allocated: true };
        };
        node.stream_ops = stream_ops;
        return node;
      },
  absolutePath() {
        abort('FS.absolutePath has been removed; use PATH_FS.resolve instead');
      },
  createFolder() {
        abort('FS.createFolder has been removed; use FS.mkdir instead');
      },
  createLink() {
        abort('FS.createLink has been removed; use FS.symlink instead');
      },
  joinPath() {
        abort('FS.joinPath has been removed; use PATH.join instead');
      },
  mmapAlloc() {
        abort('FS.mmapAlloc has been replaced by the top level function mmapAlloc');
      },
  standardizePath() {
        abort('FS.standardizePath has been removed; use PATH.normalize instead');
      },
  };
  
  var SYSCALLS = {
  DEFAULT_POLLMASK:5,
  calculateAt(dirfd, path, allowEmpty) {
        if (PATH.isAbs(path)) {
          return path;
        }
        // relative path
        var dir;
        if (dirfd === -100) {
          dir = FS.cwd();
        } else {
          var dirstream = SYSCALLS.getStreamFromFD(dirfd);
          dir = dirstream.path;
        }
        if (path.length == 0) {
          if (!allowEmpty) {
            throw new FS.ErrnoError(44);;
          }
          return dir;
        }
        return dir + '/' + path;
      },
  writeStat(buf, stat) {
        HEAP32[((buf)>>2)] = stat.dev;
        HEAP32[(((buf)+(4))>>2)] = stat.mode;
        HEAPU32[(((buf)+(8))>>2)] = stat.nlink;
        HEAP32[(((buf)+(12))>>2)] = stat.uid;
        HEAP32[(((buf)+(16))>>2)] = stat.gid;
        HEAP32[(((buf)+(20))>>2)] = stat.rdev;
        HEAP64[(((buf)+(24))>>3)] = BigInt(stat.size);
        HEAP32[(((buf)+(32))>>2)] = 4096;
        HEAP32[(((buf)+(36))>>2)] = stat.blocks;
        var atime = stat.atime.getTime();
        var mtime = stat.mtime.getTime();
        var ctime = stat.ctime.getTime();
        HEAP64[(((buf)+(40))>>3)] = BigInt(Math.floor(atime / 1000));
        HEAPU32[(((buf)+(48))>>2)] = (atime % 1000) * 1000 * 1000;
        HEAP64[(((buf)+(56))>>3)] = BigInt(Math.floor(mtime / 1000));
        HEAPU32[(((buf)+(64))>>2)] = (mtime % 1000) * 1000 * 1000;
        HEAP64[(((buf)+(72))>>3)] = BigInt(Math.floor(ctime / 1000));
        HEAPU32[(((buf)+(80))>>2)] = (ctime % 1000) * 1000 * 1000;
        HEAP64[(((buf)+(88))>>3)] = BigInt(stat.ino);
        return 0;
      },
  writeStatFs(buf, stats) {
        HEAP32[(((buf)+(4))>>2)] = stats.bsize;
        HEAP32[(((buf)+(40))>>2)] = stats.bsize;
        HEAP32[(((buf)+(8))>>2)] = stats.blocks;
        HEAP32[(((buf)+(12))>>2)] = stats.bfree;
        HEAP32[(((buf)+(16))>>2)] = stats.bavail;
        HEAP32[(((buf)+(20))>>2)] = stats.files;
        HEAP32[(((buf)+(24))>>2)] = stats.ffree;
        HEAP32[(((buf)+(28))>>2)] = stats.fsid;
        HEAP32[(((buf)+(44))>>2)] = stats.flags;  // ST_NOSUID
        HEAP32[(((buf)+(36))>>2)] = stats.namelen;
      },
  doMsync(addr, stream, len, flags, offset) {
        if (!FS.isFile(stream.node.mode)) {
          throw new FS.ErrnoError(43);
        }
        if (flags & 2) {
          // MAP_PRIVATE calls need not to be synced back to underlying fs
          return 0;
        }
        var buffer = HEAPU8.slice(addr, addr + len);
        FS.msync(stream, buffer, offset, len, flags);
      },
  getStreamFromFD(fd) {
        var stream = FS.getStreamChecked(fd);
        return stream;
      },
  varargs:undefined,
  getStr(ptr) {
        var ret = UTF8ToString(ptr);
        return ret;
      },
  };
  function ___syscall_chdir(path) {
  try {
  
      path = SYSCALLS.getStr(path);
      FS.chdir(path);
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  function ___syscall_dup3(fd, newfd, flags) {
  try {
  
      var old = SYSCALLS.getStreamFromFD(fd);
      assert(!flags);
      if (old.fd === newfd) return -28;
      // Check newfd is within range of valid open file descriptors.
      if (newfd < 0 || newfd >= FS.MAX_OPEN_FDS) return -8;
      var existing = FS.getStream(newfd);
      if (existing) FS.close(existing);
      return FS.dupStream(old, newfd).fd;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  /** @suppress {duplicate } */
  var syscallGetVarargI = () => {
      assert(SYSCALLS.varargs != undefined);
      // the `+` prepended here is necessary to convince the JSCompiler that varargs is indeed a number.
      var ret = HEAP32[((+SYSCALLS.varargs)>>2)];
      SYSCALLS.varargs += 4;
      return ret;
    };
  var syscallGetVarargP = syscallGetVarargI;
  
  
  function ___syscall_fcntl64(fd, cmd, varargs) {
  SYSCALLS.varargs = varargs;
  try {
  
      var stream = SYSCALLS.getStreamFromFD(fd);
      switch (cmd) {
        case 0: {
          var arg = syscallGetVarargI();
          if (arg < 0) {
            return -28;
          }
          while (FS.streams[arg]) {
            arg++;
          }
          var newStream;
          newStream = FS.dupStream(stream, arg);
          return newStream.fd;
        }
        case 1:
        case 2:
          return 0;  // FD_CLOEXEC makes no sense for a single process.
        case 3:
          return stream.flags;
        case 4: {
          var arg = syscallGetVarargI();
          stream.flags |= arg;
          return 0;
        }
        case 12: {
          var arg = syscallGetVarargP();
          var offset = 0;
          // We're always unlocked.
          HEAP16[(((arg)+(offset))>>1)] = 2;
          return 0;
        }
        case 13:
        case 14:
          // Pretend that the locking is successful. These are process-level locks,
          // and Emscripten programs are a single process. If we supported linking a
          // filesystem between programs, we'd need to do more here.
          // See https://github.com/emscripten-core/emscripten/issues/23697
          return 0;
      }
      return -28;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  function ___syscall_fstat64(fd, buf) {
  try {
  
      return SYSCALLS.writeStat(buf, FS.fstat(fd));
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  var INT53_MAX = 9007199254740992;
  
  var INT53_MIN = -9007199254740992;
  var bigintToI53Checked = (num) => (num < INT53_MIN || num > INT53_MAX) ? NaN : Number(num);
  function ___syscall_ftruncate64(fd, length) {
    length = bigintToI53Checked(length);
  
  
  try {
  
      if (isNaN(length)) return -61;
      FS.ftruncate(fd, length);
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  ;
  }

  
  var stringToUTF8 = (str, outPtr, maxBytesToWrite) => {
      assert(typeof maxBytesToWrite == 'number', 'stringToUTF8(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
      return stringToUTF8Array(str, HEAPU8, outPtr, maxBytesToWrite);
    };
  function ___syscall_getcwd(buf, size) {
  try {
  
      if (size === 0) return -28;
      var cwd = FS.cwd();
      var cwdLengthInBytes = lengthBytesUTF8(cwd) + 1;
      if (size < cwdLengthInBytes) return -68;
      stringToUTF8(cwd, buf, size);
      return cwdLengthInBytes;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  
  function ___syscall_getdents64(fd, dirp, count) {
  try {
  
      var stream = SYSCALLS.getStreamFromFD(fd)
      stream.getdents ||= FS.readdir(stream.path);
  
      var struct_size = 280;
      var pos = 0;
      var off = FS.llseek(stream, 0, 1);
  
      var startIdx = Math.floor(off / struct_size);
      var endIdx = Math.min(stream.getdents.length, startIdx + Math.floor(count/struct_size))
      for (var idx = startIdx; idx < endIdx; idx++) {
        var id;
        var type;
        var name = stream.getdents[idx];
        if (name === '.') {
          id = stream.node.id;
          type = 4; // DT_DIR
        }
        else if (name === '..') {
          var lookup = FS.lookupPath(stream.path, { parent: true });
          id = lookup.node.id;
          type = 4; // DT_DIR
        }
        else {
          var child;
          try {
            child = FS.lookupNode(stream.node, name);
          } catch (e) {
            // If the entry is not a directory, file, or symlink, nodefs
            // lookupNode will raise EINVAL. Skip these and continue.
            if (e?.errno === 28) {
              continue;
            }
            throw e;
          }
          id = child.id;
          type = FS.isChrdev(child.mode) ? 2 :  // DT_CHR, character device.
                 FS.isDir(child.mode) ? 4 :     // DT_DIR, directory.
                 FS.isLink(child.mode) ? 10 :   // DT_LNK, symbolic link.
                 8;                             // DT_REG, regular file.
        }
        assert(id);
        HEAP64[((dirp + pos)>>3)] = BigInt(id);
        HEAP64[(((dirp + pos)+(8))>>3)] = BigInt((idx + 1) * struct_size);
        HEAP16[(((dirp + pos)+(16))>>1)] = 280;
        HEAP8[(dirp + pos)+(18)] = type;
        stringToUTF8(name, dirp + pos + 19, 256);
        pos += struct_size;
      }
      FS.llseek(stream, idx * struct_size, 0);
      return pos;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  
  function ___syscall_ioctl(fd, op, varargs) {
  SYSCALLS.varargs = varargs;
  try {
  
      var stream = SYSCALLS.getStreamFromFD(fd);
      switch (op) {
        case 21509: {
          if (!stream.tty) return -59;
          return 0;
        }
        case 21505: {
          if (!stream.tty) return -59;
          if (stream.tty.ops.ioctl_tcgets) {
            var termios = stream.tty.ops.ioctl_tcgets(stream);
            var argp = syscallGetVarargP();
            HEAP32[((argp)>>2)] = termios.c_iflag || 0;
            HEAP32[(((argp)+(4))>>2)] = termios.c_oflag || 0;
            HEAP32[(((argp)+(8))>>2)] = termios.c_cflag || 0;
            HEAP32[(((argp)+(12))>>2)] = termios.c_lflag || 0;
            for (var i = 0; i < 32; i++) {
              HEAP8[(argp + i)+(17)] = termios.c_cc[i] || 0;
            }
            return 0;
          }
          return 0;
        }
        case 21510:
        case 21511:
        case 21512: {
          if (!stream.tty) return -59;
          return 0; // no-op, not actually adjusting terminal settings
        }
        case 21506:
        case 21507:
        case 21508: {
          if (!stream.tty) return -59;
          if (stream.tty.ops.ioctl_tcsets) {
            var argp = syscallGetVarargP();
            var c_iflag = HEAP32[((argp)>>2)];
            var c_oflag = HEAP32[(((argp)+(4))>>2)];
            var c_cflag = HEAP32[(((argp)+(8))>>2)];
            var c_lflag = HEAP32[(((argp)+(12))>>2)];
            var c_cc = []
            for (var i = 0; i < 32; i++) {
              c_cc.push(HEAP8[(argp + i)+(17)]);
            }
            return stream.tty.ops.ioctl_tcsets(stream.tty, op, { c_iflag, c_oflag, c_cflag, c_lflag, c_cc });
          }
          return 0; // no-op, not actually adjusting terminal settings
        }
        case 21519: {
          if (!stream.tty) return -59;
          var argp = syscallGetVarargP();
          HEAP32[((argp)>>2)] = 0;
          return 0;
        }
        case 21520: {
          if (!stream.tty) return -59;
          return -28; // not supported
        }
        case 21531: {
          var argp = syscallGetVarargP();
          return FS.ioctl(stream, op, argp);
        }
        case 21523: {
          // TODO: in theory we should write to the winsize struct that gets
          // passed in, but for now musl doesn't read anything on it
          if (!stream.tty) return -59;
          if (stream.tty.ops.ioctl_tiocgwinsz) {
            var winsize = stream.tty.ops.ioctl_tiocgwinsz(stream.tty);
            var argp = syscallGetVarargP();
            HEAP16[((argp)>>1)] = winsize[0];
            HEAP16[(((argp)+(2))>>1)] = winsize[1];
          }
          return 0;
        }
        case 21524: {
          // TODO: technically, this ioctl call should change the window size.
          // but, since emscripten doesn't have any concept of a terminal window
          // yet, we'll just silently throw it away as we do TIOCGWINSZ
          if (!stream.tty) return -59;
          return 0;
        }
        case 21515: {
          if (!stream.tty) return -59;
          return 0;
        }
        default: return -28; // not supported
      }
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  function ___syscall_lstat64(path, buf) {
  try {
  
      path = SYSCALLS.getStr(path);
      return SYSCALLS.writeStat(buf, FS.lstat(path));
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  function ___syscall_mkdirat(dirfd, path, mode) {
  try {
  
      path = SYSCALLS.getStr(path);
      path = SYSCALLS.calculateAt(dirfd, path);
      FS.mkdir(path, mode, 0);
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  function ___syscall_newfstatat(dirfd, path, buf, flags) {
  try {
  
      path = SYSCALLS.getStr(path);
      var nofollow = flags & 256;
      var allowEmpty = flags & 4096;
      flags = flags & (~6400);
      assert(!flags, `unknown flags in __syscall_newfstatat: ${flags}`);
      path = SYSCALLS.calculateAt(dirfd, path, allowEmpty);
      return SYSCALLS.writeStat(buf, nofollow ? FS.lstat(path) : FS.stat(path));
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  
  function ___syscall_openat(dirfd, path, flags, varargs) {
  SYSCALLS.varargs = varargs;
  try {
  
      path = SYSCALLS.getStr(path);
      path = SYSCALLS.calculateAt(dirfd, path);
      var mode = varargs ? syscallGetVarargI() : 0;
      return FS.open(path, flags, mode).fd;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  
  
  function ___syscall_readlinkat(dirfd, path, buf, bufsize) {
  try {
  
      path = SYSCALLS.getStr(path);
      path = SYSCALLS.calculateAt(dirfd, path);
      if (bufsize <= 0) return -28;
      var ret = FS.readlink(path);
  
      var len = Math.min(bufsize, lengthBytesUTF8(ret));
      var endChar = HEAP8[buf+len];
      stringToUTF8(ret, buf, bufsize+1);
      // readlink is one of the rare functions that write out a C string, but does never append a null to the output buffer(!)
      // stringToUTF8() always appends a null byte, so restore the character under the null byte after the write.
      HEAP8[buf+len] = endChar;
      return len;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  function ___syscall_rmdir(path) {
  try {
  
      path = SYSCALLS.getStr(path);
      FS.rmdir(path);
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  function ___syscall_stat64(path, buf) {
  try {
  
      path = SYSCALLS.getStr(path);
      return SYSCALLS.writeStat(buf, FS.stat(path));
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  function ___syscall_unlinkat(dirfd, path, flags) {
  try {
  
      path = SYSCALLS.getStr(path);
      path = SYSCALLS.calculateAt(dirfd, path);
      if (!flags) {
        FS.unlink(path);
      } else if (flags === 512) {
        FS.rmdir(path);
      } else {
        return -28;
      }
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  var __abort_js = () =>
      abort('native code called abort()');

  var __emscripten_throw_longjmp = () => {
      throw new EmscriptenSjLj;
    };

  var isLeapYear = (year) => year%4 === 0 && (year%100 !== 0 || year%400 === 0);
  
  var MONTH_DAYS_LEAP_CUMULATIVE = [0,31,60,91,121,152,182,213,244,274,305,335];
  
  var MONTH_DAYS_REGULAR_CUMULATIVE = [0,31,59,90,120,151,181,212,243,273,304,334];
  var ydayFromDate = (date) => {
      var leap = isLeapYear(date.getFullYear());
      var monthDaysCumulative = (leap ? MONTH_DAYS_LEAP_CUMULATIVE : MONTH_DAYS_REGULAR_CUMULATIVE);
      var yday = monthDaysCumulative[date.getMonth()] + date.getDate() - 1; // -1 since it's days since Jan 1
  
      return yday;
    };
  
  function __localtime_js(time, tmPtr) {
    time = bigintToI53Checked(time);
  
  
      var date = new Date(time*1000);
      HEAP32[((tmPtr)>>2)] = date.getSeconds();
      HEAP32[(((tmPtr)+(4))>>2)] = date.getMinutes();
      HEAP32[(((tmPtr)+(8))>>2)] = date.getHours();
      HEAP32[(((tmPtr)+(12))>>2)] = date.getDate();
      HEAP32[(((tmPtr)+(16))>>2)] = date.getMonth();
      HEAP32[(((tmPtr)+(20))>>2)] = date.getFullYear()-1900;
      HEAP32[(((tmPtr)+(24))>>2)] = date.getDay();
  
      var yday = ydayFromDate(date)|0;
      HEAP32[(((tmPtr)+(28))>>2)] = yday;
      HEAP32[(((tmPtr)+(36))>>2)] = -(date.getTimezoneOffset() * 60);
  
      // Attention: DST is in December in South, and some regions don't have DST at all.
      var start = new Date(date.getFullYear(), 0, 1);
      var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
      var winterOffset = start.getTimezoneOffset();
      var dst = (summerOffset != winterOffset && date.getTimezoneOffset() == Math.min(winterOffset, summerOffset))|0;
      HEAP32[(((tmPtr)+(32))>>2)] = dst;
    ;
  }

  
  var __mktime_js = function(tmPtr) {
  
  var ret = (() => { 
      var date = new Date(HEAP32[(((tmPtr)+(20))>>2)] + 1900,
                          HEAP32[(((tmPtr)+(16))>>2)],
                          HEAP32[(((tmPtr)+(12))>>2)],
                          HEAP32[(((tmPtr)+(8))>>2)],
                          HEAP32[(((tmPtr)+(4))>>2)],
                          HEAP32[((tmPtr)>>2)],
                          0);
  
      // There's an ambiguous hour when the time goes back; the tm_isdst field is
      // used to disambiguate it.  Date() basically guesses, so we fix it up if it
      // guessed wrong, or fill in tm_isdst with the guess if it's -1.
      var dst = HEAP32[(((tmPtr)+(32))>>2)];
      var guessedOffset = date.getTimezoneOffset();
      var start = new Date(date.getFullYear(), 0, 1);
      var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
      var winterOffset = start.getTimezoneOffset();
      var dstOffset = Math.min(winterOffset, summerOffset); // DST is in December in South
      if (dst < 0) {
        // Attention: some regions don't have DST at all.
        HEAP32[(((tmPtr)+(32))>>2)] = Number(summerOffset != winterOffset && dstOffset == guessedOffset);
      } else if ((dst > 0) != (dstOffset == guessedOffset)) {
        var nonDstOffset = Math.max(winterOffset, summerOffset);
        var trueOffset = dst > 0 ? dstOffset : nonDstOffset;
        // Don't try setMinutes(date.getMinutes() + ...) -- it's messed up.
        date.setTime(date.getTime() + (trueOffset - guessedOffset)*60000);
      }
  
      HEAP32[(((tmPtr)+(24))>>2)] = date.getDay();
      var yday = ydayFromDate(date)|0;
      HEAP32[(((tmPtr)+(28))>>2)] = yday;
      // To match expected behavior, update fields from date
      HEAP32[((tmPtr)>>2)] = date.getSeconds();
      HEAP32[(((tmPtr)+(4))>>2)] = date.getMinutes();
      HEAP32[(((tmPtr)+(8))>>2)] = date.getHours();
      HEAP32[(((tmPtr)+(12))>>2)] = date.getDate();
      HEAP32[(((tmPtr)+(16))>>2)] = date.getMonth();
      HEAP32[(((tmPtr)+(20))>>2)] = date.getYear();
  
      var timeMs = date.getTime();
      if (isNaN(timeMs)) {
        return -1;
      }
      // Return time in microseconds
      return timeMs / 1000;
     })();
  return BigInt(ret);
  };

  
  
  
  
  
  function __mmap_js(len, prot, flags, fd, offset, allocated, addr) {
    offset = bigintToI53Checked(offset);
  
  
  try {
  
      // musl's mmap doesn't allow values over a certain limit
      // see OFF_MASK in mmap.c.
      assert(!isNaN(offset));
      var stream = SYSCALLS.getStreamFromFD(fd);
      var res = FS.mmap(stream, len, offset, prot, flags);
      var ptr = res.ptr;
      HEAP32[((allocated)>>2)] = res.allocated;
      HEAPU32[((addr)>>2)] = ptr;
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  ;
  }

  
  function __munmap_js(addr, len, prot, flags, fd, offset) {
    offset = bigintToI53Checked(offset);
  
  
  try {
  
      var stream = SYSCALLS.getStreamFromFD(fd);
      if (prot & 2) {
        SYSCALLS.doMsync(addr, stream, len, flags, offset);
      }
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  ;
  }

  
  var __tzset_js = (timezone, daylight, std_name, dst_name) => {
      // TODO: Use (malleable) environment variables instead of system settings.
      var currentYear = new Date().getFullYear();
      var winter = new Date(currentYear, 0, 1);
      var summer = new Date(currentYear, 6, 1);
      var winterOffset = winter.getTimezoneOffset();
      var summerOffset = summer.getTimezoneOffset();
  
      // Local standard timezone offset. Local standard time is not adjusted for
      // daylight savings.  This code uses the fact that getTimezoneOffset returns
      // a greater value during Standard Time versus Daylight Saving Time (DST).
      // Thus it determines the expected output during Standard Time, and it
      // compares whether the output of the given date the same (Standard) or less
      // (DST).
      var stdTimezoneOffset = Math.max(winterOffset, summerOffset);
  
      // timezone is specified as seconds west of UTC ("The external variable
      // `timezone` shall be set to the difference, in seconds, between
      // Coordinated Universal Time (UTC) and local standard time."), the same
      // as returned by stdTimezoneOffset.
      // See http://pubs.opengroup.org/onlinepubs/009695399/functions/tzset.html
      HEAPU32[((timezone)>>2)] = stdTimezoneOffset * 60;
  
      HEAP32[((daylight)>>2)] = Number(winterOffset != summerOffset);
  
      var extractZone = (timezoneOffset) => {
        // Why inverse sign?
        // Read here https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date/getTimezoneOffset
        var sign = timezoneOffset >= 0 ? "-" : "+";
  
        var absOffset = Math.abs(timezoneOffset)
        var hours = String(Math.floor(absOffset / 60)).padStart(2, "0");
        var minutes = String(absOffset % 60).padStart(2, "0");
  
        return `UTC${sign}${hours}${minutes}`;
      }
  
      var winterName = extractZone(winterOffset);
      var summerName = extractZone(summerOffset);
      assert(winterName);
      assert(summerName);
      assert(lengthBytesUTF8(winterName) <= 16, `timezone name truncated to fit in TZNAME_MAX (${winterName})`);
      assert(lengthBytesUTF8(summerName) <= 16, `timezone name truncated to fit in TZNAME_MAX (${summerName})`);
      if (summerOffset < winterOffset) {
        // Northern hemisphere
        stringToUTF8(winterName, std_name, 17);
        stringToUTF8(summerName, dst_name, 17);
      } else {
        stringToUTF8(winterName, dst_name, 17);
        stringToUTF8(summerName, std_name, 17);
      }
    };

  
  var _emscripten_set_main_loop_timing = (mode, value) => {
      MainLoop.timingMode = mode;
      MainLoop.timingValue = value;
  
      if (!MainLoop.func) {
        err('emscripten_set_main_loop_timing: Cannot set timing mode for main loop since a main loop does not exist! Call emscripten_set_main_loop first to set one up.');
        return 1; // Return non-zero on failure, can't set timing mode when there is no main loop.
      }
  
      if (!MainLoop.running) {
        
        MainLoop.running = true;
      }
      if (mode == 0) {
        MainLoop.scheduler = function MainLoop_scheduler_setTimeout() {
          var timeUntilNextTick = Math.max(0, MainLoop.tickStartTime + value - _emscripten_get_now())|0;
          setTimeout(MainLoop.runner, timeUntilNextTick); // doing this each time means that on exception, we stop
        };
        MainLoop.method = 'timeout';
      } else if (mode == 1) {
        MainLoop.scheduler = function MainLoop_scheduler_rAF() {
          MainLoop.requestAnimationFrame(MainLoop.runner);
        };
        MainLoop.method = 'rAF';
      } else if (mode == 2) {
        if (typeof MainLoop.setImmediate == 'undefined') {
          if (typeof setImmediate == 'undefined') {
            // Emulate setImmediate. (note: not a complete polyfill, we don't emulate clearImmediate() to keep code size to minimum, since not needed)
            var setImmediates = [];
            var emscriptenMainLoopMessageId = 'setimmediate';
            /** @param {Event} event */
            var MainLoop_setImmediate_messageHandler = (event) => {
              // When called in current thread or Worker, the main loop ID is structured slightly different to accommodate for --proxy-to-worker runtime listening to Worker events,
              // so check for both cases.
              if (event.data === emscriptenMainLoopMessageId || event.data.target === emscriptenMainLoopMessageId) {
                event.stopPropagation();
                setImmediates.shift()();
              }
            };
            addEventListener("message", MainLoop_setImmediate_messageHandler, true);
            MainLoop.setImmediate = /** @type{function(function(): ?, ...?): number} */((func) => {
              setImmediates.push(func);
              if (ENVIRONMENT_IS_WORKER) {
                Module['setImmediates'] ??= [];
                Module['setImmediates'].push(func);
                postMessage({target: emscriptenMainLoopMessageId}); // In --proxy-to-worker, route the message via proxyClient.js
              } else postMessage(emscriptenMainLoopMessageId, "*"); // On the main thread, can just send the message to itself.
            });
          } else {
            MainLoop.setImmediate = setImmediate;
          }
        }
        MainLoop.scheduler = function MainLoop_scheduler_setImmediate() {
          MainLoop.setImmediate(MainLoop.runner);
        };
        MainLoop.method = 'immediate';
      }
      return 0;
    };
  
  var _emscripten_get_now = () => performance.now();
  
  
  var runtimeKeepaliveCounter = 0;
  var keepRuntimeAlive = () => noExitRuntime || runtimeKeepaliveCounter > 0;
  var _proc_exit = (code) => {
      EXITSTATUS = code;
      if (!keepRuntimeAlive()) {
        Module['onExit']?.(code);
        ABORT = true;
      }
      quit_(code, new ExitStatus(code));
    };
  
  
  /** @suppress {duplicate } */
  /** @param {boolean|number=} implicit */
  var exitJS = (status, implicit) => {
      EXITSTATUS = status;
  
      checkUnflushedContent();
  
      // if exit() was called explicitly, warn the user if the runtime isn't actually being shut down
      if (keepRuntimeAlive() && !implicit) {
        var msg = `program exited (with status: ${status}), but keepRuntimeAlive() is set (counter=${runtimeKeepaliveCounter}) due to an async operation, so halting execution but not exiting the runtime or preventing further async execution (you can use emscripten_force_exit, if you want to force a true shutdown)`;
        err(msg);
      }
  
      _proc_exit(status);
    };
  var _exit = exitJS;
  
  var handleException = (e) => {
      // Certain exception types we do not treat as errors since they are used for
      // internal control flow.
      // 1. ExitStatus, which is thrown by exit()
      // 2. "unwind", which is thrown by emscripten_unwind_to_js_event_loop() and others
      //    that wish to return to JS event loop.
      if (e instanceof ExitStatus || e == 'unwind') {
        return EXITSTATUS;
      }
      checkStackCookie();
      if (e instanceof WebAssembly.RuntimeError) {
        if (_emscripten_stack_get_current() <= 0) {
          err('Stack overflow detected.  You can try increasing -sSTACK_SIZE (currently set to 500000)');
        }
      }
      quit_(1, e);
    };
  
  var maybeExit = () => {
      if (!keepRuntimeAlive()) {
        try {
          _exit(EXITSTATUS);
        } catch (e) {
          handleException(e);
        }
      }
    };
  
    /**
     * @param {number=} arg
     * @param {boolean=} noSetTiming
     */
  var setMainLoop = (iterFunc, fps, simulateInfiniteLoop, arg, noSetTiming) => {
      assert(!MainLoop.func, 'emscripten_set_main_loop: there can only be one main loop function at once: call emscripten_cancel_main_loop to cancel the previous one before setting a new one with different parameters.');
      MainLoop.func = iterFunc;
      MainLoop.arg = arg;
  
      var thisMainLoopId = MainLoop.currentlyRunningMainloop;
      function checkIsRunning() {
        if (thisMainLoopId < MainLoop.currentlyRunningMainloop) {
          
          maybeExit();
          return false;
        }
        return true;
      }
  
      // We create the loop runner here but it is not actually running until
      // _emscripten_set_main_loop_timing is called (which might happen a
      // later time).  This member signifies that the current runner has not
      // yet been started so that we can call runtimeKeepalivePush when it
      // gets it timing set for the first time.
      MainLoop.running = false;
      MainLoop.runner = function MainLoop_runner() {
        if (ABORT) return;
        if (MainLoop.queue.length > 0) {
          var start = Date.now();
          var blocker = MainLoop.queue.shift();
          blocker.func(blocker.arg);
          if (MainLoop.remainingBlockers) {
            var remaining = MainLoop.remainingBlockers;
            var next = remaining%1 == 0 ? remaining-1 : Math.floor(remaining);
            if (blocker.counted) {
              MainLoop.remainingBlockers = next;
            } else {
              // not counted, but move the progress along a tiny bit
              next = next + 0.5; // do not steal all the next one's progress
              MainLoop.remainingBlockers = (8*remaining + next)/9;
            }
          }
          MainLoop.updateStatus();
  
          // catches pause/resume main loop from blocker execution
          if (!checkIsRunning()) return;
  
          setTimeout(MainLoop.runner, 0);
          return;
        }
  
        // catch pauses from non-main loop sources
        if (!checkIsRunning()) return;
  
        // Implement very basic swap interval control
        MainLoop.currentFrameNumber = MainLoop.currentFrameNumber + 1 | 0;
        if (MainLoop.timingMode == 1 && MainLoop.timingValue > 1 && MainLoop.currentFrameNumber % MainLoop.timingValue != 0) {
          // Not the scheduled time to render this frame - skip.
          MainLoop.scheduler();
          return;
        } else if (MainLoop.timingMode == 0) {
          MainLoop.tickStartTime = _emscripten_get_now();
        }
  
        if (MainLoop.method === 'timeout' && Module['ctx']) {
          warnOnce('Looks like you are rendering without using requestAnimationFrame for the main loop. You should use 0 for the frame rate in emscripten_set_main_loop in order to use requestAnimationFrame, as that can greatly improve your frame rates!');
          MainLoop.method = ''; // just warn once per call to set main loop
        }
  
        MainLoop.runIter(iterFunc);
  
        // catch pauses from the main loop itself
        if (!checkIsRunning()) return;
  
        MainLoop.scheduler();
      }
  
      if (!noSetTiming) {
        if (fps > 0) {
          _emscripten_set_main_loop_timing(0, 1000.0 / fps);
        } else {
          // Do rAF by rendering each frame (no decimating)
          _emscripten_set_main_loop_timing(1, 1);
        }
  
        MainLoop.scheduler();
      }
  
      if (simulateInfiniteLoop) {
        throw 'unwind';
      }
    };
  
  
  var callUserCallback = (func) => {
      if (ABORT) {
        err('user callback triggered after runtime exited or application aborted.  Ignoring.');
        return;
      }
      try {
        func();
        maybeExit();
      } catch (e) {
        handleException(e);
      }
    };
  
  var MainLoop = {
  running:false,
  scheduler:null,
  method:"",
  currentlyRunningMainloop:0,
  func:null,
  arg:0,
  timingMode:0,
  timingValue:0,
  currentFrameNumber:0,
  queue:[],
  preMainLoop:[],
  postMainLoop:[],
  pause() {
        MainLoop.scheduler = null;
        // Incrementing this signals the previous main loop that it's now become old, and it must return.
        MainLoop.currentlyRunningMainloop++;
      },
  resume() {
        MainLoop.currentlyRunningMainloop++;
        var timingMode = MainLoop.timingMode;
        var timingValue = MainLoop.timingValue;
        var func = MainLoop.func;
        MainLoop.func = null;
        // do not set timing and call scheduler, we will do it on the next lines
        setMainLoop(func, 0, false, MainLoop.arg, true);
        _emscripten_set_main_loop_timing(timingMode, timingValue);
        MainLoop.scheduler();
      },
  updateStatus() {
        if (Module['setStatus']) {
          var message = Module['statusMessage'] || 'Please wait...';
          var remaining = MainLoop.remainingBlockers ?? 0;
          var expected = MainLoop.expectedBlockers ?? 0;
          if (remaining) {
            if (remaining < expected) {
              Module['setStatus'](`{message} ({expected - remaining}/{expected})`);
            } else {
              Module['setStatus'](message);
            }
          } else {
            Module['setStatus']('');
          }
        }
      },
  init() {
        Module['preMainLoop'] && MainLoop.preMainLoop.push(Module['preMainLoop']);
        Module['postMainLoop'] && MainLoop.postMainLoop.push(Module['postMainLoop']);
      },
  runIter(func) {
        if (ABORT) return;
        for (var pre of MainLoop.preMainLoop) {
          if (pre() === false) {
            return; // |return false| skips a frame
          }
        }
        callUserCallback(func);
        for (var post of MainLoop.postMainLoop) {
          post();
        }
        checkStackCookie();
      },
  nextRAF:0,
  fakeRequestAnimationFrame(func) {
        // try to keep 60fps between calls to here
        var now = Date.now();
        if (MainLoop.nextRAF === 0) {
          MainLoop.nextRAF = now + 1000/60;
        } else {
          while (now + 2 >= MainLoop.nextRAF) { // fudge a little, to avoid timer jitter causing us to do lots of delay:0
            MainLoop.nextRAF += 1000/60;
          }
        }
        var delay = Math.max(MainLoop.nextRAF - now, 0);
        setTimeout(func, delay);
      },
  requestAnimationFrame(func) {
        if (typeof requestAnimationFrame == 'function') {
          requestAnimationFrame(func);
          return;
        }
        var RAF = MainLoop.fakeRequestAnimationFrame;
        RAF(func);
      },
  };
  
  var AL = {
  QUEUE_INTERVAL:25,
  QUEUE_LOOKAHEAD:0.1,
  DEVICE_NAME:"Emscripten OpenAL",
  CAPTURE_DEVICE_NAME:"Emscripten OpenAL capture",
  ALC_EXTENSIONS:{
  ALC_SOFT_pause_device:true,
  ALC_SOFT_HRTF:true,
  },
  AL_EXTENSIONS:{
  AL_EXT_float32:true,
  AL_SOFT_loop_points:true,
  AL_SOFT_source_length:true,
  AL_EXT_source_distance_model:true,
  AL_SOFT_source_spatialize:true,
  },
  _alcErr:0,
  alcErr:0,
  deviceRefCounts:{
  },
  alcStringCache:{
  },
  paused:false,
  stringCache:{
  },
  contexts:{
  },
  currentCtx:null,
  buffers:{
  0:{
  id:0,
  refCount:0,
  audioBuf:null,
  frequency:0,
  bytesPerSample:2,
  channels:1,
  length:0,
  },
  },
  paramArray:[],
  _nextId:1,
  newId:() => AL.freeIds.length > 0 ? AL.freeIds.pop() : AL._nextId++,
  freeIds:[],
  scheduleContextAudio:(ctx) => {
        // If we are animating using the requestAnimationFrame method, then the main loop does not run when in the background.
        // To give a perfect glitch-free audio stop when switching from foreground to background, we need to avoid updating
        // audio altogether when in the background, so detect that case and kill audio buffer streaming if so.
        if (MainLoop.timingMode === 1 && document['visibilityState'] != 'visible') {
          return;
        }
  
        for (var i in ctx.sources) {
          AL.scheduleSourceAudio(ctx.sources[i]);
        }
      },
  scheduleSourceAudio:(src, lookahead) => {
        // See comment on scheduleContextAudio above.
        if (MainLoop.timingMode === 1 && document['visibilityState'] != 'visible') {
          return;
        }
        if (src.state !== 4114) {
          return;
        }
  
        var currentTime = AL.updateSourceTime(src);
  
        var startTime = src.bufStartTime;
        var startOffset = src.bufOffset;
        var bufCursor = src.bufsProcessed;
  
        // Advance past any audio that is already scheduled
        for (var i = 0; i < src.audioQueue.length; i++) {
          var audioSrc = src.audioQueue[i];
          startTime = audioSrc._startTime + audioSrc._duration;
          startOffset = 0.0;
          bufCursor += audioSrc._skipCount + 1;
        }
  
        if (!lookahead) {
          lookahead = AL.QUEUE_LOOKAHEAD;
        }
        var lookaheadTime = currentTime + lookahead;
        var skipCount = 0;
        while (startTime < lookaheadTime) {
          if (bufCursor >= src.bufQueue.length) {
            if (src.looping) {
              bufCursor %= src.bufQueue.length;
            } else {
              break;
            }
          }
  
          var buf = src.bufQueue[bufCursor % src.bufQueue.length];
          // If the buffer contains no data, skip it
          if (buf.length === 0) {
            skipCount++;
            // If we've gone through the whole queue and everything is 0 length, just give up
            if (skipCount === src.bufQueue.length) {
              break;
            }
          } else {
            var audioSrc = src.context.audioCtx.createBufferSource();
            audioSrc.buffer = buf.audioBuf;
            audioSrc.playbackRate.value = src.playbackRate;
            if (buf.audioBuf._loopStart || buf.audioBuf._loopEnd) {
              audioSrc.loopStart = buf.audioBuf._loopStart;
              audioSrc.loopEnd = buf.audioBuf._loopEnd;
            }
  
            var duration = 0.0;
            // If the source is a looping static buffer, use native looping for gapless playback
            if (src.type === 4136 && src.looping) {
              duration = Number.POSITIVE_INFINITY;
              audioSrc.loop = true;
              if (buf.audioBuf._loopStart) {
                audioSrc.loopStart = buf.audioBuf._loopStart;
              }
              if (buf.audioBuf._loopEnd) {
                audioSrc.loopEnd = buf.audioBuf._loopEnd;
              }
            } else {
              duration = (buf.audioBuf.duration - startOffset) / src.playbackRate;
            }
  
            audioSrc._startOffset = startOffset;
            audioSrc._duration = duration;
            audioSrc._skipCount = skipCount;
            skipCount = 0;
  
            audioSrc.connect(src.gain);
  
            if (typeof audioSrc.start != 'undefined') {
              // Sample the current time as late as possible to mitigate drift
              startTime = Math.max(startTime, src.context.audioCtx.currentTime);
              audioSrc.start(startTime, startOffset);
            } else if (typeof audioSrc.noteOn != 'undefined') {
              startTime = Math.max(startTime, src.context.audioCtx.currentTime);
              audioSrc.noteOn(startTime);
            }
            audioSrc._startTime = startTime;
            src.audioQueue.push(audioSrc);
  
            startTime += duration;
          }
  
          startOffset = 0.0;
          bufCursor++;
        }
      },
  updateSourceTime:(src) => {
        var currentTime = src.context.audioCtx.currentTime;
        if (src.state !== 4114) {
          return currentTime;
        }
  
        // if the start time is unset, determine it based on the current offset.
        // This will be the case when a source is resumed after being paused, and
        // allows us to pretend that the source actually started playing some time
        // in the past such that it would just now have reached the stored offset.
        if (!isFinite(src.bufStartTime)) {
          src.bufStartTime = currentTime - src.bufOffset / src.playbackRate;
          src.bufOffset = 0.0;
        }
  
        var nextStartTime = 0.0;
        while (src.audioQueue.length) {
          var audioSrc = src.audioQueue[0];
          src.bufsProcessed += audioSrc._skipCount;
          nextStartTime = audioSrc._startTime + audioSrc._duration; // n.b. audioSrc._duration already factors in playbackRate, so no divide by src.playbackRate on it.
  
          if (currentTime < nextStartTime) {
            break;
          }
  
          src.audioQueue.shift();
          src.bufStartTime = nextStartTime;
          src.bufOffset = 0.0;
          src.bufsProcessed++;
        }
  
        if (src.bufsProcessed >= src.bufQueue.length && !src.looping) {
          // The source has played its entire queue and is non-looping, so just mark it as stopped.
          AL.setSourceState(src, 4116);
        } else if (src.type === 4136 && src.looping) {
          // If the source is a looping static buffer, determine the buffer offset based on the loop points
          var buf = src.bufQueue[0];
          if (buf.length === 0) {
            src.bufOffset = 0.0;
          } else {
            var delta = (currentTime - src.bufStartTime) * src.playbackRate;
            var loopStart = buf.audioBuf._loopStart || 0.0;
            var loopEnd = buf.audioBuf._loopEnd || buf.audioBuf.duration;
            if (loopEnd <= loopStart) {
              loopEnd = buf.audioBuf.duration;
            }
  
            if (delta < loopEnd) {
              src.bufOffset = delta;
            } else {
              src.bufOffset = loopStart + (delta - loopStart) % (loopEnd - loopStart);
            }
          }
        } else if (src.audioQueue[0]) {
          // The source is still actively playing, so we just need to calculate where we are in the current buffer
          // so it can be remembered if the source gets paused.
          src.bufOffset = (currentTime - src.audioQueue[0]._startTime) * src.playbackRate;
        } else {
          // The source hasn't finished yet, but there is no scheduled audio left for it. This can be because
          // the source has just been started/resumed, or due to an underrun caused by a long blocking operation.
          // We need to determine what state we would be in by this point in time so that when we next schedule
          // audio playback, it will be just as if no underrun occurred.
  
          if (src.type !== 4136 && src.looping) {
            // if the source is a looping buffer queue, let's first calculate the queue duration, so we can
            // quickly fast forward past any full loops of the queue and only worry about the remainder.
            var srcDuration = AL.sourceDuration(src) / src.playbackRate;
            if (srcDuration > 0.0) {
              src.bufStartTime += Math.floor((currentTime - src.bufStartTime) / srcDuration) * srcDuration;
            }
          }
  
          // Since we've already skipped any full-queue loops if there were any, we just need to find
          // out where in the queue the remaining time puts us, which won't require stepping through the
          // entire queue more than once.
          for (var i = 0; i < src.bufQueue.length; i++) {
            if (src.bufsProcessed >= src.bufQueue.length) {
              if (src.looping) {
                src.bufsProcessed %= src.bufQueue.length;
              } else {
                AL.setSourceState(src, 4116);
                break;
              }
            }
  
            var buf = src.bufQueue[src.bufsProcessed];
            if (buf.length > 0) {
              nextStartTime = src.bufStartTime + buf.audioBuf.duration / src.playbackRate;
  
              if (currentTime < nextStartTime) {
                src.bufOffset = (currentTime - src.bufStartTime) * src.playbackRate;
                break;
              }
  
              src.bufStartTime = nextStartTime;
            }
  
            src.bufOffset = 0.0;
            src.bufsProcessed++;
          }
        }
  
        return currentTime;
      },
  cancelPendingSourceAudio:(src) => {
        AL.updateSourceTime(src);
  
        for (var i = 1; i < src.audioQueue.length; i++) {
          var audioSrc = src.audioQueue[i];
          audioSrc.stop();
        }
  
        if (src.audioQueue.length > 1) {
          src.audioQueue.length = 1;
        }
      },
  stopSourceAudio:(src) => {
        for (var i = 0; i < src.audioQueue.length; i++) {
          src.audioQueue[i].stop();
        }
        src.audioQueue.length = 0;
      },
  setSourceState:(src, state) => {
        if (state === 4114) {
          if (src.state === 4114 || src.state == 4116) {
            src.bufsProcessed = 0;
            src.bufOffset = 0.0;
          } else {
          }
  
          AL.stopSourceAudio(src);
  
          src.state = 4114;
          src.bufStartTime = Number.NEGATIVE_INFINITY;
          AL.scheduleSourceAudio(src);
        } else if (state === 4115) {
          if (src.state === 4114) {
            // Store off the current offset to restore with on resume.
            AL.updateSourceTime(src);
            AL.stopSourceAudio(src);
  
            src.state = 4115;
          }
        } else if (state === 4116) {
          if (src.state !== 4113) {
            src.state = 4116;
            src.bufsProcessed = src.bufQueue.length;
            src.bufStartTime = Number.NEGATIVE_INFINITY;
            src.bufOffset = 0.0;
            AL.stopSourceAudio(src);
          }
        } else if (state === 4113) {
          if (src.state !== 4113) {
            src.state = 4113;
            src.bufsProcessed = 0;
            src.bufStartTime = Number.NEGATIVE_INFINITY;
            src.bufOffset = 0.0;
            AL.stopSourceAudio(src);
          }
        }
      },
  initSourcePanner:(src) => {
        if (src.type === 0x1030 /* AL_UNDETERMINED */) {
          return;
        }
  
        // Find the first non-zero buffer in the queue to determine the proper format
        var templateBuf = AL.buffers[0];
        for (var i = 0; i < src.bufQueue.length; i++) {
          if (src.bufQueue[i].id !== 0) {
            templateBuf = src.bufQueue[i];
            break;
          }
        }
        // Create a panner if AL_SOURCE_SPATIALIZE_SOFT is set to true, or alternatively if it's set to auto and the source is mono
        if (src.spatialize === 1 || (src.spatialize === 2 /* AL_AUTO_SOFT */ && templateBuf.channels === 1)) {
          if (src.panner) {
            return;
          }
          src.panner = src.context.audioCtx.createPanner();
  
          AL.updateSourceGlobal(src);
          AL.updateSourceSpace(src);
  
          src.panner.connect(src.context.gain);
          src.gain.disconnect();
          src.gain.connect(src.panner);
        } else {
          if (!src.panner) {
            return;
          }
  
          src.panner.disconnect();
          src.gain.disconnect();
          src.gain.connect(src.context.gain);
          src.panner = null;
        }
      },
  updateContextGlobal:(ctx) => {
        for (var i in ctx.sources) {
          AL.updateSourceGlobal(ctx.sources[i]);
        }
      },
  updateSourceGlobal:(src) => {
        var panner = src.panner;
        if (!panner) {
          return;
        }
  
        panner.refDistance = src.refDistance;
        panner.maxDistance = src.maxDistance;
        panner.rolloffFactor = src.rolloffFactor;
  
        panner.panningModel = src.context.hrtf ? 'HRTF' : 'equalpower';
  
        // Use the source's distance model if AL_SOURCE_DISTANCE_MODEL is enabled
        var distanceModel = src.context.sourceDistanceModel ? src.distanceModel : src.context.distanceModel;
        switch (distanceModel) {
        case 0:
          panner.distanceModel = 'inverse';
          panner.refDistance = 3.40282e38 /* FLT_MAX */;
          break;
        case 0xd001 /* AL_INVERSE_DISTANCE */:
        case 0xd002 /* AL_INVERSE_DISTANCE_CLAMPED */:
          panner.distanceModel = 'inverse';
          break;
        case 0xd003 /* AL_LINEAR_DISTANCE */:
        case 0xd004 /* AL_LINEAR_DISTANCE_CLAMPED */:
          panner.distanceModel = 'linear';
          break;
        case 0xd005 /* AL_EXPONENT_DISTANCE */:
        case 0xd006 /* AL_EXPONENT_DISTANCE_CLAMPED */:
          panner.distanceModel = 'exponential';
          break;
        }
      },
  updateListenerSpace:(ctx) => {
        var listener = ctx.audioCtx.listener;
        if (listener.positionX) {
          listener.positionX.value = ctx.listener.position[0];
          listener.positionY.value = ctx.listener.position[1];
          listener.positionZ.value = ctx.listener.position[2];
        } else {
          listener.setPosition(ctx.listener.position[0], ctx.listener.position[1], ctx.listener.position[2]);
        }
        if (listener.forwardX) {
          listener.forwardX.value = ctx.listener.direction[0];
          listener.forwardY.value = ctx.listener.direction[1];
          listener.forwardZ.value = ctx.listener.direction[2];
          listener.upX.value = ctx.listener.up[0];
          listener.upY.value = ctx.listener.up[1];
          listener.upZ.value = ctx.listener.up[2];
        } else {
          listener.setOrientation(
            ctx.listener.direction[0], ctx.listener.direction[1], ctx.listener.direction[2],
            ctx.listener.up[0], ctx.listener.up[1], ctx.listener.up[2]);
        }
  
        // Update sources that are relative to the listener
        for (var i in ctx.sources) {
          AL.updateSourceSpace(ctx.sources[i]);
        }
      },
  updateSourceSpace:(src) => {
        if (!src.panner) {
          return;
        }
        var panner = src.panner;
  
        var posX = src.position[0];
        var posY = src.position[1];
        var posZ = src.position[2];
        var dirX = src.direction[0];
        var dirY = src.direction[1];
        var dirZ = src.direction[2];
  
        var listener = src.context.listener;
        var lPosX = listener.position[0];
        var lPosY = listener.position[1];
        var lPosZ = listener.position[2];
  
        // WebAudio does spatialization in world-space coordinates, meaning both the buffer sources and
        // the listener position are in the same absolute coordinate system relative to a fixed origin.
        // By default, OpenAL works this way as well, but it also provides a "listener relative" mode, where
        // a buffer source's coordinate are interpreted not in absolute world space, but as being relative
        // to the listener object itself, so as the listener moves the source appears to move with it
        // with no update required. Since web audio does not support this mode, we must transform the source
        // coordinates from listener-relative space to absolute world space.
        //
        // We do this via affine transformation matrices applied to the source position and source direction.
        // A change-of-basis converts from listener-space displacements to world-space displacements,
        // which must be done for both the source position and direction. Lastly, the source position must be
        // added to the listener position to get the final source position, since the source position represents
        // a displacement from the listener.
        if (src.relative) {
          // Negate the listener direction since forward is -Z.
          var lBackX = -listener.direction[0];
          var lBackY = -listener.direction[1];
          var lBackZ = -listener.direction[2];
          var lUpX = listener.up[0];
          var lUpY = listener.up[1];
          var lUpZ = listener.up[2];
  
          var inverseMagnitude = (x, y, z) => {
            var length = Math.sqrt(x * x + y * y + z * z);
  
            if (length < Number.EPSILON) {
              return 0.0;
            }
  
            return 1.0 / length;
          };
  
          // Normalize the Back vector
          var invMag = inverseMagnitude(lBackX, lBackY, lBackZ);
          lBackX *= invMag;
          lBackY *= invMag;
          lBackZ *= invMag;
  
          // ...and the Up vector
          invMag = inverseMagnitude(lUpX, lUpY, lUpZ);
          lUpX *= invMag;
          lUpY *= invMag;
          lUpZ *= invMag;
  
          // Calculate the Right vector as the cross product of the Up and Back vectors
          var lRightX = (lUpY * lBackZ - lUpZ * lBackY);
          var lRightY = (lUpZ * lBackX - lUpX * lBackZ);
          var lRightZ = (lUpX * lBackY - lUpY * lBackX);
  
          // Back and Up might not be exactly perpendicular, so the cross product also needs normalization
          invMag = inverseMagnitude(lRightX, lRightY, lRightZ);
          lRightX *= invMag;
          lRightY *= invMag;
          lRightZ *= invMag;
  
          // Recompute Up from the now orthonormal Right and Back vectors so we have a fully orthonormal basis
          lUpX = (lBackY * lRightZ - lBackZ * lRightY);
          lUpY = (lBackZ * lRightX - lBackX * lRightZ);
          lUpZ = (lBackX * lRightY - lBackY * lRightX);
  
          var oldX = dirX;
          var oldY = dirY;
          var oldZ = dirZ;
  
          // Use our 3 vectors to apply a change-of-basis matrix to the source direction
          dirX = oldX * lRightX + oldY * lUpX + oldZ * lBackX;
          dirY = oldX * lRightY + oldY * lUpY + oldZ * lBackY;
          dirZ = oldX * lRightZ + oldY * lUpZ + oldZ * lBackZ;
  
          oldX = posX;
          oldY = posY;
          oldZ = posZ;
  
          // ...and to the source position
          posX = oldX * lRightX + oldY * lUpX + oldZ * lBackX;
          posY = oldX * lRightY + oldY * lUpY + oldZ * lBackY;
          posZ = oldX * lRightZ + oldY * lUpZ + oldZ * lBackZ;
  
          // The change-of-basis corrects the orientation, but the origin is still the listener.
          // Translate the source position by the listener position to finish.
          posX += lPosX;
          posY += lPosY;
          posZ += lPosZ;
        }
  
        if (panner.positionX) {
          // Assigning to panner.positionX/Y/Z unnecessarily seems to cause performance issues
          // See https://github.com/emscripten-core/emscripten/issues/15847
  
          if (posX != panner.positionX.value) panner.positionX.value = posX;
          if (posY != panner.positionY.value) panner.positionY.value = posY;
          if (posZ != panner.positionZ.value) panner.positionZ.value = posZ;
        } else {
          panner.setPosition(posX, posY, posZ);
        }
        if (panner.orientationX) {
          // Assigning to panner.orientation/Y/Z unnecessarily seems to cause performance issues
          // See https://github.com/emscripten-core/emscripten/issues/15847
  
          if (dirX != panner.orientationX.value) panner.orientationX.value = dirX;
          if (dirY != panner.orientationY.value) panner.orientationY.value = dirY;
          if (dirZ != panner.orientationZ.value) panner.orientationZ.value = dirZ;
        } else {
          panner.setOrientation(dirX, dirY, dirZ);
        }
  
        var oldShift = src.dopplerShift;
        var velX = src.velocity[0];
        var velY = src.velocity[1];
        var velZ = src.velocity[2];
        var lVelX = listener.velocity[0];
        var lVelY = listener.velocity[1];
        var lVelZ = listener.velocity[2];
        if (posX === lPosX && posY === lPosY && posZ === lPosZ
          || velX === lVelX && velY === lVelY && velZ === lVelZ)
        {
          src.dopplerShift = 1.0;
        } else {
          // Doppler algorithm from 1.1 spec
          var speedOfSound = src.context.speedOfSound;
          var dopplerFactor = src.context.dopplerFactor;
  
          var slX = lPosX - posX;
          var slY = lPosY - posY;
          var slZ = lPosZ - posZ;
  
          var magSl = Math.sqrt(slX * slX + slY * slY + slZ * slZ);
          var vls = (slX * lVelX + slY * lVelY + slZ * lVelZ) / magSl;
          var vss = (slX * velX + slY * velY + slZ * velZ) / magSl;
  
          vls = Math.min(vls, speedOfSound / dopplerFactor);
          vss = Math.min(vss, speedOfSound / dopplerFactor);
  
          src.dopplerShift = (speedOfSound - dopplerFactor * vls) / (speedOfSound - dopplerFactor * vss);
        }
        if (src.dopplerShift !== oldShift) {
          AL.updateSourceRate(src);
        }
      },
  updateSourceRate:(src) => {
        if (src.state === 4114) {
          // clear scheduled buffers
          AL.cancelPendingSourceAudio(src);
  
          var audioSrc = src.audioQueue[0];
          if (!audioSrc) {
            return; // It is possible that AL.scheduleContextAudio() has not yet fed the next buffer, if so, skip.
          }
  
          var duration;
          if (src.type === 4136 && src.looping) {
            duration = Number.POSITIVE_INFINITY;
          } else {
            // audioSrc._duration is expressed after factoring in playbackRate, so when changing playback rate, need
            // to recompute/rescale the rate to the new playback speed.
            duration = (audioSrc.buffer.duration - audioSrc._startOffset) / src.playbackRate;
          }
  
          audioSrc._duration = duration;
          audioSrc.playbackRate.value = src.playbackRate;
  
          // reschedule buffers with the new playbackRate
          AL.scheduleSourceAudio(src);
        }
      },
  sourceDuration:(src) => {
        var length = 0.0;
        for (var i = 0; i < src.bufQueue.length; i++) {
          var audioBuf = src.bufQueue[i].audioBuf;
          length += audioBuf ? audioBuf.duration : 0.0;
        }
        return length;
      },
  sourceTell:(src) => {
        AL.updateSourceTime(src);
  
        var offset = 0.0;
        for (var i = 0; i < src.bufsProcessed; i++) {
          if (src.bufQueue[i].audioBuf) {
            offset += src.bufQueue[i].audioBuf.duration;
          }
        }
        offset += src.bufOffset;
  
        return offset;
      },
  sourceSeek:(src, offset) => {
        var playing = src.state == 4114;
        if (playing) {
          AL.setSourceState(src, 4113);
        }
  
        if (src.bufQueue[src.bufsProcessed].audioBuf !== null) {
          src.bufsProcessed = 0;
          while (offset > src.bufQueue[src.bufsProcessed].audioBuf.duration) {
            offset -= src.bufQueue[src.bufsProcessed].audioBuf.duration;
            src.bufsProcessed++;
          }
  
          src.bufOffset = offset;
        }
  
        if (playing) {
          AL.setSourceState(src, 4114);
        }
      },
  getGlobalParam:(funcname, param) => {
        if (!AL.currentCtx) {
          return null;
        }
  
        switch (param) {
        case 49152:
          return AL.currentCtx.dopplerFactor;
        case 49155:
          return AL.currentCtx.speedOfSound;
        case 53248:
          return AL.currentCtx.distanceModel;
        default:
          AL.currentCtx.err = 40962;
          return null;
        }
      },
  setGlobalParam:(funcname, param, value) => {
        if (!AL.currentCtx) {
          return;
        }
  
        switch (param) {
        case 49152:
          if (!Number.isFinite(value) || value < 0.0) { // Strictly negative values are disallowed
            AL.currentCtx.err = 40963;
            return;
          }
  
          AL.currentCtx.dopplerFactor = value;
          AL.updateListenerSpace(AL.currentCtx);
          break;
        case 49155:
          if (!Number.isFinite(value) || value <= 0.0) { // Negative or zero values are disallowed
            AL.currentCtx.err = 40963;
            return;
          }
  
          AL.currentCtx.speedOfSound = value;
          AL.updateListenerSpace(AL.currentCtx);
          break;
        case 53248:
          switch (value) {
          case 0:
          case 0xd001 /* AL_INVERSE_DISTANCE */:
          case 0xd002 /* AL_INVERSE_DISTANCE_CLAMPED */:
          case 0xd003 /* AL_LINEAR_DISTANCE */:
          case 0xd004 /* AL_LINEAR_DISTANCE_CLAMPED */:
          case 0xd005 /* AL_EXPONENT_DISTANCE */:
          case 0xd006 /* AL_EXPONENT_DISTANCE_CLAMPED */:
            AL.currentCtx.distanceModel = value;
            AL.updateContextGlobal(AL.currentCtx);
            break;
          default:
            AL.currentCtx.err = 40963;
            return;
          }
          break;
        default:
          AL.currentCtx.err = 40962;
          return;
        }
      },
  getListenerParam:(funcname, param) => {
        if (!AL.currentCtx) {
          return null;
        }
  
        switch (param) {
        case 4100:
          return AL.currentCtx.listener.position;
        case 4102:
          return AL.currentCtx.listener.velocity;
        case 4111:
          return AL.currentCtx.listener.direction.concat(AL.currentCtx.listener.up);
        case 4106:
          return AL.currentCtx.gain.gain.value;
        default:
          AL.currentCtx.err = 40962;
          return null;
        }
      },
  setListenerParam:(funcname, param, value) => {
        if (!AL.currentCtx) {
          return;
        }
        if (value === null) {
          AL.currentCtx.err = 40962;
          return;
        }
  
        var listener = AL.currentCtx.listener;
        switch (param) {
        case 4100:
          if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          listener.position[0] = value[0];
          listener.position[1] = value[1];
          listener.position[2] = value[2];
          AL.updateListenerSpace(AL.currentCtx);
          break;
        case 4102:
          if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          listener.velocity[0] = value[0];
          listener.velocity[1] = value[1];
          listener.velocity[2] = value[2];
          AL.updateListenerSpace(AL.currentCtx);
          break;
        case 4106:
          if (!Number.isFinite(value) || value < 0.0) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          AL.currentCtx.gain.gain.value = value;
          break;
        case 4111:
          if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])
            || !Number.isFinite(value[3]) || !Number.isFinite(value[4]) || !Number.isFinite(value[5])
          ) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          listener.direction[0] = value[0];
          listener.direction[1] = value[1];
          listener.direction[2] = value[2];
          listener.up[0] = value[3];
          listener.up[1] = value[4];
          listener.up[2] = value[5];
          AL.updateListenerSpace(AL.currentCtx);
          break;
        default:
          AL.currentCtx.err = 40962;
          return;
        }
      },
  getBufferParam:(funcname, bufferId, param) => {
        if (!AL.currentCtx) {
          return;
        }
        var buf = AL.buffers[bufferId];
        if (!buf || bufferId === 0) {
          AL.currentCtx.err = 40961;
          return;
        }
  
        switch (param) {
        case 0x2001 /* AL_FREQUENCY */:
          return buf.frequency;
        case 0x2002 /* AL_BITS */:
          return buf.bytesPerSample * 8;
        case 0x2003 /* AL_CHANNELS */:
          return buf.channels;
        case 0x2004 /* AL_SIZE */:
          return buf.length * buf.bytesPerSample * buf.channels;
        case 0x2015 /* AL_LOOP_POINTS_SOFT */:
          if (buf.length === 0) {
            return [0, 0];
          }
          return [
            (buf.audioBuf._loopStart || 0.0) * buf.frequency,
            (buf.audioBuf._loopEnd || buf.length) * buf.frequency
          ];
        default:
          AL.currentCtx.err = 40962;
          return null;
        }
      },
  setBufferParam:(funcname, bufferId, param, value) => {
        if (!AL.currentCtx) {
          return;
        }
        var buf = AL.buffers[bufferId];
        if (!buf || bufferId === 0) {
          AL.currentCtx.err = 40961;
          return;
        }
        if (value === null) {
          AL.currentCtx.err = 40962;
          return;
        }
  
        switch (param) {
        case 0x2004 /* AL_SIZE */:
          if (value !== 0) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          // Per the spec, setting AL_SIZE to 0 is a legal NOP.
          break;
        case 0x2015 /* AL_LOOP_POINTS_SOFT */:
          if (value[0] < 0 || value[0] > buf.length || value[1] < 0 || value[1] > buf.Length || value[0] >= value[1]) {
            AL.currentCtx.err = 40963;
            return;
          }
          if (buf.refCount > 0) {
            AL.currentCtx.err = 40964;
            return;
          }
  
          if (buf.audioBuf) {
            buf.audioBuf._loopStart = value[0] / buf.frequency;
            buf.audioBuf._loopEnd = value[1] / buf.frequency;
          }
          break;
        default:
          AL.currentCtx.err = 40962;
          return;
        }
      },
  getSourceParam:(funcname, sourceId, param) => {
        if (!AL.currentCtx) {
          return null;
        }
        var src = AL.currentCtx.sources[sourceId];
        if (!src) {
          AL.currentCtx.err = 40961;
          return null;
        }
  
        switch (param) {
        case 0x202 /* AL_SOURCE_RELATIVE */:
          return src.relative;
        case 0x1001 /* AL_CONE_INNER_ANGLE */:
          return src.coneInnerAngle;
        case 0x1002 /* AL_CONE_OUTER_ANGLE */:
          return src.coneOuterAngle;
        case 0x1003 /* AL_PITCH */:
          return src.pitch;
        case 4100:
          return src.position;
        case 4101:
          return src.direction;
        case 4102:
          return src.velocity;
        case 0x1007 /* AL_LOOPING */:
          return src.looping;
        case 0x1009 /* AL_BUFFER */:
          if (src.type === 4136) {
            return src.bufQueue[0].id;
          }
          return 0;
        case 4106:
          return src.gain.gain.value;
         case 0x100D /* AL_MIN_GAIN */:
          return src.minGain;
        case 0x100E /* AL_MAX_GAIN */:
          return src.maxGain;
        case 0x1010 /* AL_SOURCE_STATE */:
          return src.state;
        case 0x1015 /* AL_BUFFERS_QUEUED */:
          if (src.bufQueue.length === 1 && src.bufQueue[0].id === 0) {
            return 0;
          }
          return src.bufQueue.length;
        case 0x1016 /* AL_BUFFERS_PROCESSED */:
          if ((src.bufQueue.length === 1 && src.bufQueue[0].id === 0) || src.looping) {
            return 0;
          }
          return src.bufsProcessed;
        case 0x1020 /* AL_REFERENCE_DISTANCE */:
          return src.refDistance;
        case 0x1021 /* AL_ROLLOFF_FACTOR */:
          return src.rolloffFactor;
        case 0x1022 /* AL_CONE_OUTER_GAIN */:
          return src.coneOuterGain;
        case 0x1023 /* AL_MAX_DISTANCE */:
          return src.maxDistance;
        case 0x1024 /* AL_SEC_OFFSET */:
          return AL.sourceTell(src);
        case 0x1025 /* AL_SAMPLE_OFFSET */:
          var offset = AL.sourceTell(src);
          if (offset > 0.0) {
            offset *= src.bufQueue[0].frequency;
          }
          return offset;
        case 0x1026 /* AL_BYTE_OFFSET */:
          var offset = AL.sourceTell(src);
          if (offset > 0.0) {
            offset *= src.bufQueue[0].frequency * src.bufQueue[0].bytesPerSample;
          }
          return offset;
        case 0x1027 /* AL_SOURCE_TYPE */:
          return src.type;
        case 0x1214 /* AL_SOURCE_SPATIALIZE_SOFT */:
          return src.spatialize;
        case 0x2009 /* AL_BYTE_LENGTH_SOFT */:
          var length = 0;
          var bytesPerFrame = 0;
          for (var i = 0; i < src.bufQueue.length; i++) {
            length += src.bufQueue[i].length;
            if (src.bufQueue[i].id !== 0) {
              bytesPerFrame = src.bufQueue[i].bytesPerSample * src.bufQueue[i].channels;
            }
          }
          return length * bytesPerFrame;
        case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
          var length = 0;
          for (var i = 0; i < src.bufQueue.length; i++) {
            length += src.bufQueue[i].length;
          }
          return length;
        case 0x200B /* AL_SEC_LENGTH_SOFT */:
          return AL.sourceDuration(src);
        case 53248:
          return src.distanceModel;
        default:
          AL.currentCtx.err = 40962;
          return null;
        }
      },
  setSourceParam:(funcname, sourceId, param, value) => {
        if (!AL.currentCtx) {
          return;
        }
        var src = AL.currentCtx.sources[sourceId];
        if (!src) {
          AL.currentCtx.err = 40961;
          return;
        }
        if (value === null) {
          AL.currentCtx.err = 40962;
          return;
        }
  
        switch (param) {
        case 0x202 /* AL_SOURCE_RELATIVE */:
          if (value === 1) {
            src.relative = true;
            AL.updateSourceSpace(src);
          } else if (value === 0) {
            src.relative = false;
            AL.updateSourceSpace(src);
          } else {
            AL.currentCtx.err = 40963;
            return;
          }
          break;
        case 0x1001 /* AL_CONE_INNER_ANGLE */:
          if (!Number.isFinite(value)) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          src.coneInnerAngle = value;
          if (src.panner) {
            src.panner.coneInnerAngle = value % 360.0;
          }
          break;
        case 0x1002 /* AL_CONE_OUTER_ANGLE */:
          if (!Number.isFinite(value)) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          src.coneOuterAngle = value;
          if (src.panner) {
            src.panner.coneOuterAngle = value % 360.0;
          }
          break;
        case 0x1003 /* AL_PITCH */:
          if (!Number.isFinite(value) || value <= 0.0) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          if (src.pitch === value) {
            break;
          }
  
          src.pitch = value;
          AL.updateSourceRate(src);
          break;
        case 4100:
          if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          src.position[0] = value[0];
          src.position[1] = value[1];
          src.position[2] = value[2];
          AL.updateSourceSpace(src);
          break;
        case 4101:
          if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          src.direction[0] = value[0];
          src.direction[1] = value[1];
          src.direction[2] = value[2];
          AL.updateSourceSpace(src);
          break;
        case 4102:
          if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          src.velocity[0] = value[0];
          src.velocity[1] = value[1];
          src.velocity[2] = value[2];
          AL.updateSourceSpace(src);
          break;
        case 0x1007 /* AL_LOOPING */:
          if (value === 1) {
            src.looping = true;
            AL.updateSourceTime(src);
            if (src.type === 4136 && src.audioQueue.length > 0) {
              var audioSrc  = src.audioQueue[0];
              audioSrc.loop = true;
              audioSrc._duration = Number.POSITIVE_INFINITY;
            }
          } else if (value === 0) {
            src.looping = false;
            var currentTime = AL.updateSourceTime(src);
            if (src.type === 4136 && src.audioQueue.length > 0) {
              var audioSrc  = src.audioQueue[0];
              audioSrc.loop = false;
              audioSrc._duration = src.bufQueue[0].audioBuf.duration / src.playbackRate;
              audioSrc._startTime = currentTime - src.bufOffset / src.playbackRate;
            }
          } else {
            AL.currentCtx.err = 40963;
            return;
          }
          break;
        case 0x1009 /* AL_BUFFER */:
          if (src.state === 4114 || src.state === 4115) {
            AL.currentCtx.err = 40964;
            return;
          }
  
          if (value === 0) {
            for (var i in src.bufQueue) {
              src.bufQueue[i].refCount--;
            }
            src.bufQueue.length = 1;
            src.bufQueue[0] = AL.buffers[0];
  
            src.bufsProcessed = 0;
            src.type = 0x1030 /* AL_UNDETERMINED */;
          } else {
            var buf = AL.buffers[value];
            if (!buf) {
              AL.currentCtx.err = 40963;
              return;
            }
  
            for (var i in src.bufQueue) {
              src.bufQueue[i].refCount--;
            }
            src.bufQueue.length = 0;
  
            buf.refCount++;
            src.bufQueue = [buf];
            src.bufsProcessed = 0;
            src.type = 4136;
          }
  
          AL.initSourcePanner(src);
          AL.scheduleSourceAudio(src);
          break;
        case 4106:
          if (!Number.isFinite(value) || value < 0.0) {
            AL.currentCtx.err = 40963;
            return;
          }
          src.gain.gain.value = value;
          break;
        case 0x100D /* AL_MIN_GAIN */:
          if (!Number.isFinite(value) || value < 0.0 || value > Math.min(src.maxGain, 1.0)) {
            AL.currentCtx.err = 40963;
            return;
          }
          src.minGain = value;
          break;
        case 0x100E /* AL_MAX_GAIN */:
          if (!Number.isFinite(value) || value < Math.max(0.0, src.minGain) || value > 1.0) {
            AL.currentCtx.err = 40963;
            return;
          }
          src.maxGain = value;
          break;
        case 0x1020 /* AL_REFERENCE_DISTANCE */:
          if (!Number.isFinite(value) || value < 0.0) {
            AL.currentCtx.err = 40963;
            return;
          }
          src.refDistance = value;
          if (src.panner) {
            src.panner.refDistance = value;
          }
          break;
        case 0x1021 /* AL_ROLLOFF_FACTOR */:
          if (!Number.isFinite(value) || value < 0.0) {
            AL.currentCtx.err = 40963;
            return;
          }
          src.rolloffFactor = value;
          if (src.panner) {
            src.panner.rolloffFactor = value;
          }
          break;
        case 0x1022 /* AL_CONE_OUTER_GAIN */:
          if (!Number.isFinite(value) || value < 0.0 || value > 1.0) {
            AL.currentCtx.err = 40963;
            return;
          }
          src.coneOuterGain = value;
          if (src.panner) {
            src.panner.coneOuterGain = value;
          }
          break;
        case 0x1023 /* AL_MAX_DISTANCE */:
          if (!Number.isFinite(value) || value < 0.0) {
            AL.currentCtx.err = 40963;
            return;
          }
          src.maxDistance = value;
          if (src.panner) {
            src.panner.maxDistance = value;
          }
          break;
        case 0x1024 /* AL_SEC_OFFSET */:
          if (value < 0.0 || value > AL.sourceDuration(src)) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          AL.sourceSeek(src, value);
          break;
        case 0x1025 /* AL_SAMPLE_OFFSET */:
          var srcLen = AL.sourceDuration(src);
          if (srcLen > 0.0) {
            var frequency;
            for (var bufId in src.bufQueue) {
              if (bufId) {
                frequency = src.bufQueue[bufId].frequency;
                break;
              }
            }
            value /= frequency;
          }
          if (value < 0.0 || value > srcLen) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          AL.sourceSeek(src, value);
          break;
        case 0x1026 /* AL_BYTE_OFFSET */:
          var srcLen = AL.sourceDuration(src);
          if (srcLen > 0.0) {
            var bytesPerSec;
            for (var bufId in src.bufQueue) {
              if (bufId) {
                var buf = src.bufQueue[bufId];
                bytesPerSec = buf.frequency * buf.bytesPerSample * buf.channels;
                break;
              }
            }
            value /= bytesPerSec;
          }
          if (value < 0.0 || value > srcLen) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          AL.sourceSeek(src, value);
          break;
        case 0x1214 /* AL_SOURCE_SPATIALIZE_SOFT */:
          if (value !== 0 && value !== 1 && value !== 2 /* AL_AUTO_SOFT */) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          src.spatialize = value;
          AL.initSourcePanner(src);
          break;
        case 0x2009 /* AL_BYTE_LENGTH_SOFT */:
        case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
        case 0x200B /* AL_SEC_LENGTH_SOFT */:
          AL.currentCtx.err = 40964;
          break;
        case 53248:
          switch (value) {
          case 0:
          case 0xd001 /* AL_INVERSE_DISTANCE */:
          case 0xd002 /* AL_INVERSE_DISTANCE_CLAMPED */:
          case 0xd003 /* AL_LINEAR_DISTANCE */:
          case 0xd004 /* AL_LINEAR_DISTANCE_CLAMPED */:
          case 0xd005 /* AL_EXPONENT_DISTANCE */:
          case 0xd006 /* AL_EXPONENT_DISTANCE_CLAMPED */:
            src.distanceModel = value;
            if (AL.currentCtx.sourceDistanceModel) {
              AL.updateContextGlobal(AL.currentCtx);
            }
            break;
          default:
            AL.currentCtx.err = 40963;
            return;
          }
          break;
        default:
          AL.currentCtx.err = 40962;
          return;
        }
      },
  captures:{
  },
  sharedCaptureAudioCtx:null,
  requireValidCaptureDevice:(deviceId, funcname) => {
        if (deviceId === 0) {
          AL.alcErr = 40961;
          return null;
        }
        var c = AL.captures[deviceId];
        if (!c) {
          AL.alcErr = 40961;
          return null;
        }
        var err = c.mediaStreamError;
        if (err) {
          AL.alcErr = 40961;
          return null;
        }
        return c;
      },
  };
  var _alBufferData = (bufferId, format, pData, size, freq) => {
      if (!AL.currentCtx) {
        return;
      }
      var buf = AL.buffers[bufferId];
      if (!buf) {
        AL.currentCtx.err = 40963;
        return;
      }
      if (freq <= 0) {
        AL.currentCtx.err = 40963;
        return;
      }
  
      var audioBuf = null;
      try {
        switch (format) {
        case 0x1100 /* AL_FORMAT_MONO8 */:
          if (size > 0) {
            audioBuf = AL.currentCtx.audioCtx.createBuffer(1, size, freq);
            var channel0 = audioBuf.getChannelData(0);
            for (var i = 0; i < size; ++i) {
              channel0[i] = HEAPU8[pData++] * 0.0078125 /* 1/128 */ - 1.0;
            }
          }
          buf.bytesPerSample = 1;
          buf.channels = 1;
          buf.length = size;
          break;
        case 0x1101 /* AL_FORMAT_MONO16 */:
          if (size > 0) {
            audioBuf = AL.currentCtx.audioCtx.createBuffer(1, size >> 1, freq);
            var channel0 = audioBuf.getChannelData(0);
            pData >>= 1;
            for (var i = 0; i < size >> 1; ++i) {
              channel0[i] = HEAP16[pData++] * 0.000030517578125 /* 1/32768 */;
            }
          }
          buf.bytesPerSample = 2;
          buf.channels = 1;
          buf.length = size >> 1;
          break;
        case 0x1102 /* AL_FORMAT_STEREO8 */:
          if (size > 0) {
            audioBuf = AL.currentCtx.audioCtx.createBuffer(2, size >> 1, freq);
            var channel0 = audioBuf.getChannelData(0);
            var channel1 = audioBuf.getChannelData(1);
            for (var i = 0; i < size >> 1; ++i) {
              channel0[i] = HEAPU8[pData++] * 0.0078125 /* 1/128 */ - 1.0;
              channel1[i] = HEAPU8[pData++] * 0.0078125 /* 1/128 */ - 1.0;
            }
          }
          buf.bytesPerSample = 1;
          buf.channels = 2;
          buf.length = size >> 1;
          break;
        case 0x1103 /* AL_FORMAT_STEREO16 */:
          if (size > 0) {
            audioBuf = AL.currentCtx.audioCtx.createBuffer(2, size >> 2, freq);
            var channel0 = audioBuf.getChannelData(0);
            var channel1 = audioBuf.getChannelData(1);
            pData >>= 1;
            for (var i = 0; i < size >> 2; ++i) {
              channel0[i] = HEAP16[pData++] * 0.000030517578125 /* 1/32768 */;
              channel1[i] = HEAP16[pData++] * 0.000030517578125 /* 1/32768 */;
            }
          }
          buf.bytesPerSample = 2;
          buf.channels = 2;
          buf.length = size >> 2;
          break;
        case 0x10010 /* AL_FORMAT_MONO_FLOAT32 */:
          if (size > 0) {
            audioBuf = AL.currentCtx.audioCtx.createBuffer(1, size >> 2, freq);
            var channel0 = audioBuf.getChannelData(0);
            pData >>= 2;
            for (var i = 0; i < size >> 2; ++i) {
              channel0[i] = HEAPF32[pData++];
            }
          }
          buf.bytesPerSample = 4;
          buf.channels = 1;
          buf.length = size >> 2;
          break;
        case 0x10011 /* AL_FORMAT_STEREO_FLOAT32 */:
          if (size > 0) {
            audioBuf = AL.currentCtx.audioCtx.createBuffer(2, size >> 3, freq);
            var channel0 = audioBuf.getChannelData(0);
            var channel1 = audioBuf.getChannelData(1);
            pData >>= 2;
            for (var i = 0; i < size >> 3; ++i) {
              channel0[i] = HEAPF32[pData++];
              channel1[i] = HEAPF32[pData++];
            }
          }
          buf.bytesPerSample = 4;
          buf.channels = 2;
          buf.length = size >> 3;
          break;
        default:
          AL.currentCtx.err = 40963;
          return;
        }
        buf.frequency = freq;
        buf.audioBuf = audioBuf;
      } catch (e) {
        AL.currentCtx.err = 40963;
        return;
      }
    };

  var _alDeleteBuffers = (count, pBufferIds) => {
      if (!AL.currentCtx) {
        return;
      }
  
      for (var i = 0; i < count; ++i) {
        var bufId = HEAP32[(((pBufferIds)+(i*4))>>2)];
        /// Deleting the zero buffer is a legal NOP, so ignore it
        if (bufId === 0) {
          continue;
        }
  
        // Make sure the buffer index is valid.
        if (!AL.buffers[bufId]) {
          AL.currentCtx.err = 40961;
          return;
        }
  
        // Make sure the buffer is no longer in use.
        if (AL.buffers[bufId].refCount) {
          AL.currentCtx.err = 40964;
          return;
        }
      }
  
      for (var i = 0; i < count; ++i) {
        var bufId = HEAP32[(((pBufferIds)+(i*4))>>2)];
        if (bufId === 0) {
          continue;
        }
  
        AL.deviceRefCounts[AL.buffers[bufId].deviceId]--;
        delete AL.buffers[bufId];
        AL.freeIds.push(bufId);
      }
    };

  var _alSourcei = (sourceId, param, value) => {
      switch (param) {
      case 0x202 /* AL_SOURCE_RELATIVE */:
      case 0x1001 /* AL_CONE_INNER_ANGLE */:
      case 0x1002 /* AL_CONE_OUTER_ANGLE */:
      case 0x1007 /* AL_LOOPING */:
      case 0x1009 /* AL_BUFFER */:
      case 0x1020 /* AL_REFERENCE_DISTANCE */:
      case 0x1021 /* AL_ROLLOFF_FACTOR */:
      case 0x1023 /* AL_MAX_DISTANCE */:
      case 0x1024 /* AL_SEC_OFFSET */:
      case 0x1025 /* AL_SAMPLE_OFFSET */:
      case 0x1026 /* AL_BYTE_OFFSET */:
      case 0x1214 /* AL_SOURCE_SPATIALIZE_SOFT */:
      case 0x2009 /* AL_BYTE_LENGTH_SOFT */:
      case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
      case 53248:
        AL.setSourceParam('alSourcei', sourceId, param, value);
        break;
      default:
        AL.setSourceParam('alSourcei', sourceId, param, null);
        break;
      }
    };
  
  var _alDeleteSources = (count, pSourceIds) => {
      if (!AL.currentCtx) {
        return;
      }
  
      for (var i = 0; i < count; ++i) {
        var srcId = HEAP32[(((pSourceIds)+(i*4))>>2)];
        if (!AL.currentCtx.sources[srcId]) {
          AL.currentCtx.err = 40961;
          return;
        }
      }
  
      for (var i = 0; i < count; ++i) {
        var srcId = HEAP32[(((pSourceIds)+(i*4))>>2)];
        AL.setSourceState(AL.currentCtx.sources[srcId], 4116);
        _alSourcei(srcId, 0x1009 /* AL_BUFFER */, 0);
        delete AL.currentCtx.sources[srcId];
        AL.freeIds.push(srcId);
      }
    };

  var _alDistanceModel = (model) => {
      AL.setGlobalParam('alDistanceModel', 53248, model);
    };

  var _alGenBuffers = (count, pBufferIds) => {
      if (!AL.currentCtx) {
        return;
      }
  
      for (var i = 0; i < count; ++i) {
        var buf = {
          deviceId: AL.currentCtx.deviceId,
          id: AL.newId(),
          refCount: 0,
          audioBuf: null,
          frequency: 0,
          bytesPerSample: 2,
          channels: 1,
          length: 0,
        };
        AL.deviceRefCounts[buf.deviceId]++;
        AL.buffers[buf.id] = buf;
        HEAP32[(((pBufferIds)+(i*4))>>2)] = buf.id;
      }
    };

  var _alGenSources = (count, pSourceIds) => {
      if (!AL.currentCtx) {
        return;
      }
      for (var i = 0; i < count; ++i) {
        var gain = AL.currentCtx.audioCtx.createGain();
        gain.connect(AL.currentCtx.gain);
        var src = {
          context: AL.currentCtx,
          id: AL.newId(),
          type: 0x1030 /* AL_UNDETERMINED */,
          state: 4113,
          bufQueue: [AL.buffers[0]],
          audioQueue: [],
          looping: false,
          pitch: 1.0,
          dopplerShift: 1.0,
          gain,
          minGain: 0.0,
          maxGain: 1.0,
          panner: null,
          bufsProcessed: 0,
          bufStartTime: Number.NEGATIVE_INFINITY,
          bufOffset: 0.0,
          relative: false,
          refDistance: 1.0,
          maxDistance: 3.40282e38 /* FLT_MAX */,
          rolloffFactor: 1.0,
          position: [0.0, 0.0, 0.0],
          velocity: [0.0, 0.0, 0.0],
          direction: [0.0, 0.0, 0.0],
          coneOuterGain: 0.0,
          coneInnerAngle: 360.0,
          coneOuterAngle: 360.0,
          distanceModel: 0xd002 /* AL_INVERSE_DISTANCE_CLAMPED */,
          spatialize: 2 /* AL_AUTO_SOFT */,
  
          get playbackRate() {
            return this.pitch * this.dopplerShift;
          }
        };
        AL.currentCtx.sources[src.id] = src;
        HEAP32[(((pSourceIds)+(i*4))>>2)] = src.id;
      }
    };

  var _alGetError = () => {
      if (!AL.currentCtx) {
        return 40964;
      }
      // Reset error on get.
      var err = AL.currentCtx.err;
      AL.currentCtx.err = 0;
      return err;
    };

  var _alGetListenerfv = (param, pValues) => {
      var val = AL.getListenerParam('alGetListenerfv', param);
      if (val === null) {
        return;
      }
      if (!pValues) {
        AL.currentCtx.err = 40963;
        return;
      }
  
      switch (param) {
      case 4100:
      case 4102:
        HEAPF32[((pValues)>>2)] = val[0];
        HEAPF32[(((pValues)+(4))>>2)] = val[1];
        HEAPF32[(((pValues)+(8))>>2)] = val[2];
        break;
      case 4111:
        HEAPF32[((pValues)>>2)] = val[0];
        HEAPF32[(((pValues)+(4))>>2)] = val[1];
        HEAPF32[(((pValues)+(8))>>2)] = val[2];
        HEAPF32[(((pValues)+(12))>>2)] = val[3];
        HEAPF32[(((pValues)+(16))>>2)] = val[4];
        HEAPF32[(((pValues)+(20))>>2)] = val[5];
        break;
      default:
        AL.currentCtx.err = 40962;
        return;
      }
    };

  var _alGetSourcei = (sourceId, param, pValue) => {
      var val = AL.getSourceParam('alGetSourcei', sourceId, param);
      if (val === null) {
        return;
      }
      if (!pValue) {
        AL.currentCtx.err = 40963;
        return;
      }
  
      switch (param) {
      case 0x202 /* AL_SOURCE_RELATIVE */:
      case 0x1001 /* AL_CONE_INNER_ANGLE */:
      case 0x1002 /* AL_CONE_OUTER_ANGLE */:
      case 0x1007 /* AL_LOOPING */:
      case 0x1009 /* AL_BUFFER */:
      case 0x1010 /* AL_SOURCE_STATE */:
      case 0x1015 /* AL_BUFFERS_QUEUED */:
      case 0x1016 /* AL_BUFFERS_PROCESSED */:
      case 0x1020 /* AL_REFERENCE_DISTANCE */:
      case 0x1021 /* AL_ROLLOFF_FACTOR */:
      case 0x1023 /* AL_MAX_DISTANCE */:
      case 0x1024 /* AL_SEC_OFFSET */:
      case 0x1025 /* AL_SAMPLE_OFFSET */:
      case 0x1026 /* AL_BYTE_OFFSET */:
      case 0x1027 /* AL_SOURCE_TYPE */:
      case 0x1214 /* AL_SOURCE_SPATIALIZE_SOFT */:
      case 0x2009 /* AL_BYTE_LENGTH_SOFT */:
      case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
      case 53248:
        HEAP32[((pValue)>>2)] = val;
        break;
      default:
        AL.currentCtx.err = 40962;
        return;
      }
    };

  
  
  var stringToNewUTF8 = (str) => {
      var size = lengthBytesUTF8(str) + 1;
      var ret = _malloc(size);
      if (ret) stringToUTF8(str, ret, size);
      return ret;
    };
  
  var _alGetString = (param) => {
      if (AL.stringCache[param]) {
        return AL.stringCache[param];
      }
  
      var ret;
      switch (param) {
      case 0:
        ret = 'No Error';
        break;
      case 40961:
        ret = 'Invalid Name';
        break;
      case 40962:
        ret = 'Invalid Enum';
        break;
      case 40963:
        ret = 'Invalid Value';
        break;
      case 40964:
        ret = 'Invalid Operation';
        break;
      case 0xA005 /* AL_OUT_OF_MEMORY */:
        ret = 'Out of Memory';
        break;
      case 0xB001 /* AL_VENDOR */:
        ret = 'Emscripten';
        break;
      case 0xB002 /* AL_VERSION */:
        ret = '1.1';
        break;
      case 0xB003 /* AL_RENDERER */:
        ret = 'WebAudio';
        break;
      case 0xB004 /* AL_EXTENSIONS */:
        ret = Object.keys(AL.AL_EXTENSIONS).join(' ');
        break;
      default:
        if (AL.currentCtx) {
          AL.currentCtx.err = 40962;
        } else {
        }
        return 0;
      }
  
      ret = stringToNewUTF8(ret);
      AL.stringCache[param] = ret;
      return ret;
    };

  
  var _alIsExtensionPresent = (pExtName) => {
      var name = UTF8ToString(pExtName);
  
      return AL.AL_EXTENSIONS[name] ? 1 : 0;
    };

  var _alListener3f = (param, value0, value1, value2) => {
      switch (param) {
      case 4100:
      case 4102:
        AL.paramArray[0] = value0;
        AL.paramArray[1] = value1;
        AL.paramArray[2] = value2;
        AL.setListenerParam('alListener3f', param, AL.paramArray);
        break;
      default:
        AL.setListenerParam('alListener3f', param, null);
        break;
      }
    };

  var _alListenerfv = (param, pValues) => {
      if (!AL.currentCtx) {
        return;
      }
      if (!pValues) {
        AL.currentCtx.err = 40963;
        return;
      }
  
      switch (param) {
      case 4100:
      case 4102:
        AL.paramArray[0] = HEAPF32[((pValues)>>2)];
        AL.paramArray[1] = HEAPF32[(((pValues)+(4))>>2)];
        AL.paramArray[2] = HEAPF32[(((pValues)+(8))>>2)];
        AL.setListenerParam('alListenerfv', param, AL.paramArray);
        break;
      case 4111:
        AL.paramArray[0] = HEAPF32[((pValues)>>2)];
        AL.paramArray[1] = HEAPF32[(((pValues)+(4))>>2)];
        AL.paramArray[2] = HEAPF32[(((pValues)+(8))>>2)];
        AL.paramArray[3] = HEAPF32[(((pValues)+(12))>>2)];
        AL.paramArray[4] = HEAPF32[(((pValues)+(16))>>2)];
        AL.paramArray[5] = HEAPF32[(((pValues)+(20))>>2)];
        AL.setListenerParam('alListenerfv', param, AL.paramArray);
        break;
      default:
        AL.setListenerParam('alListenerfv', param, null);
        break;
      }
    };

  var _alSource3f = (sourceId, param, value0, value1, value2) => {
      switch (param) {
      case 4100:
      case 4101:
      case 4102:
        AL.paramArray[0] = value0;
        AL.paramArray[1] = value1;
        AL.paramArray[2] = value2;
        AL.setSourceParam('alSource3f', sourceId, param, AL.paramArray);
        break;
      default:
        AL.setSourceParam('alSource3f', sourceId, param, null);
        break;
      }
    };

  var _alSourcePlay = (sourceId) => {
      if (!AL.currentCtx) {
        return;
      }
      var src = AL.currentCtx.sources[sourceId];
      if (!src) {
        AL.currentCtx.err = 40961;
        return;
      }
      AL.setSourceState(src, 4114);
    };

  var _alSourceQueueBuffers = (sourceId, count, pBufferIds) => {
      if (!AL.currentCtx) {
        return;
      }
      var src = AL.currentCtx.sources[sourceId];
      if (!src) {
        AL.currentCtx.err = 40961;
        return;
      }
      if (src.type === 4136) {
        AL.currentCtx.err = 40964;
        return;
      }
  
      if (count === 0) {
        return;
      }
  
      // Find the first non-zero buffer in the queue to determine the proper format
      var templateBuf = AL.buffers[0];
      for (var buf of src.bufQueue) {
        if (buf.id !== 0) {
          templateBuf = buf;
          break;
        }
      }
  
      for (var i = 0; i < count; ++i) {
        var bufId = HEAP32[(((pBufferIds)+(i*4))>>2)];
        var buf = AL.buffers[bufId];
        if (!buf) {
          AL.currentCtx.err = 40961;
          return;
        }
  
        // Check that the added buffer has the correct format. If the template is the zero buffer, any format is valid.
        if (templateBuf.id !== 0 && (
          buf.frequency !== templateBuf.frequency
          || buf.bytesPerSample !== templateBuf.bytesPerSample
          || buf.channels !== templateBuf.channels)
        ) {
          AL.currentCtx.err = 40964;
        }
      }
  
      // If the only buffer in the queue is the zero buffer, clear the queue before we add anything.
      if (src.bufQueue.length === 1 && src.bufQueue[0].id === 0) {
        src.bufQueue.length = 0;
      }
  
      src.type = 0x1029 /* AL_STREAMING */;
      for (var i = 0; i < count; ++i) {
        var bufId = HEAP32[(((pBufferIds)+(i*4))>>2)];
        var buf = AL.buffers[bufId];
        buf.refCount++;
        src.bufQueue.push(buf);
      }
  
      // if the source is looping, cancel the schedule so we can reschedule the loop order
      if (src.looping) {
        AL.cancelPendingSourceAudio(src);
      }
  
      AL.initSourcePanner(src);
      AL.scheduleSourceAudio(src);
    };

  var _alSourceStop = (sourceId) => {
      if (!AL.currentCtx) {
        return;
      }
      var src = AL.currentCtx.sources[sourceId];
      if (!src) {
        AL.currentCtx.err = 40961;
        return;
      }
      AL.setSourceState(src, 4116);
    };

  var _alSourceUnqueueBuffers = (sourceId, count, pBufferIds) => {
      if (!AL.currentCtx) {
        return;
      }
      var src = AL.currentCtx.sources[sourceId];
      if (!src) {
        AL.currentCtx.err = 40961;
        return;
      }
      if (count > (src.bufQueue.length === 1 && src.bufQueue[0].id === 0 ? 0 : src.bufsProcessed)) {
        AL.currentCtx.err = 40963;
        return;
      }
  
      if (count === 0) {
        return;
      }
  
      for (var i = 0; i < count; i++) {
        var buf = src.bufQueue.shift();
        buf.refCount--;
        // Write the buffers index out to the return list.
        HEAP32[(((pBufferIds)+(i*4))>>2)] = buf.id;
        src.bufsProcessed--;
      }
  
      /// If the queue is empty, put the zero buffer back in
      if (src.bufQueue.length === 0) {
        src.bufQueue.push(AL.buffers[0]);
      }
  
      AL.initSourcePanner(src);
      AL.scheduleSourceAudio(src);
    };

  var _alSourcef = (sourceId, param, value) => {
      switch (param) {
      case 0x1001 /* AL_CONE_INNER_ANGLE */:
      case 0x1002 /* AL_CONE_OUTER_ANGLE */:
      case 0x1003 /* AL_PITCH */:
      case 4106:
      case 0x100D /* AL_MIN_GAIN */:
      case 0x100E /* AL_MAX_GAIN */:
      case 0x1020 /* AL_REFERENCE_DISTANCE */:
      case 0x1021 /* AL_ROLLOFF_FACTOR */:
      case 0x1022 /* AL_CONE_OUTER_GAIN */:
      case 0x1023 /* AL_MAX_DISTANCE */:
      case 0x1024 /* AL_SEC_OFFSET */:
      case 0x1025 /* AL_SAMPLE_OFFSET */:
      case 0x1026 /* AL_BYTE_OFFSET */:
      case 0x200B /* AL_SEC_LENGTH_SOFT */:
        AL.setSourceParam('alSourcef', sourceId, param, value);
        break;
      default:
        AL.setSourceParam('alSourcef', sourceId, param, null);
        break;
      }
    };


  var _alcCloseDevice = (deviceId) => {
      if (!(deviceId in AL.deviceRefCounts) || AL.deviceRefCounts[deviceId] > 0) {
        return 0;
      }
  
      delete AL.deviceRefCounts[deviceId];
      AL.freeIds.push(deviceId);
      return 1;
    };

  var listenOnce = (object, event, func) =>
      object.addEventListener(event, func, { 'once': true });
  /** @param {Object=} elements */
  var autoResumeAudioContext = (ctx, elements) => {
      if (!elements) {
        elements = [document, document.getElementById('canvas')];
      }
      ['keydown', 'mousedown', 'touchstart'].forEach((event) => {
        elements.forEach((element) => {
          if (element) {
            listenOnce(element, event, () => {
              if (ctx.state === 'suspended') ctx.resume();
            });
          }
        });
      });
    };
  
  var _alcCreateContext = (deviceId, pAttrList) => {
      if (!(deviceId in AL.deviceRefCounts)) {
        AL.alcErr = 0xA001; /* ALC_INVALID_DEVICE */
        return 0;
      }
  
      var options = null;
      var attrs = [];
      var hrtf = null;
      pAttrList >>= 2;
      if (pAttrList) {
        var attr = 0;
        var val = 0;
        while (true) {
          attr = HEAP32[pAttrList++];
          attrs.push(attr);
          if (attr === 0) {
            break;
          }
          val = HEAP32[pAttrList++];
          attrs.push(val);
  
          switch (attr) {
          case 0x1007 /* ALC_FREQUENCY */:
            if (!options) {
              options = {};
            }
  
            options.sampleRate = val;
            break;
          case 0x1010 /* ALC_MONO_SOURCES */: // fallthrough
          case 0x1011 /* ALC_STEREO_SOURCES */:
            // Do nothing; these hints are satisfied by default
            break
          case 0x1992 /* ALC_HRTF_SOFT */:
            switch (val) {
              case 0:
                hrtf = false;
                break;
              case 1:
                hrtf = true;
                break;
              case 2 /* ALC_DONT_CARE_SOFT */:
                break;
              default:
                AL.alcErr = 40964;
                return 0;
            }
            break;
          case 0x1996 /* ALC_HRTF_ID_SOFT */:
            if (val !== 0) {
              AL.alcErr = 40964;
              return 0;
            }
            break;
          default:
            AL.alcErr = 0xA004; /* ALC_INVALID_VALUE */
            return 0;
          }
        }
      }
  
      var AudioContext = window.AudioContext || window.webkitAudioContext;
      var ac = null;
      try {
        // Only try to pass options if there are any, for compat with browsers that don't support this
        if (options) {
          ac = new AudioContext(options);
        } else {
          ac = new AudioContext();
        }
      } catch (e) {
        if (e.name === 'NotSupportedError') {
          AL.alcErr = 0xA004; /* ALC_INVALID_VALUE */
        } else {
          AL.alcErr = 0xA001; /* ALC_INVALID_DEVICE */
        }
  
        return 0;
      }
  
      autoResumeAudioContext(ac);
  
      // Old Web Audio API (e.g. Safari 6.0.5) had an inconsistently named createGainNode function.
      if (typeof ac.createGain == 'undefined') {
        ac.createGain = ac.createGainNode;
      }
  
      var gain = ac.createGain();
      gain.connect(ac.destination);
      var ctx = {
        deviceId,
        id: AL.newId(),
        attrs,
        audioCtx: ac,
        listener: {
          position: [0.0, 0.0, 0.0],
          velocity: [0.0, 0.0, 0.0],
          direction: [0.0, 0.0, 0.0],
          up: [0.0, 0.0, 0.0]
        },
        sources: [],
        interval: setInterval(() => AL.scheduleContextAudio(ctx), AL.QUEUE_INTERVAL),
        gain,
        distanceModel: 0xd002 /* AL_INVERSE_DISTANCE_CLAMPED */,
        speedOfSound: 343.3,
        dopplerFactor: 1.0,
        sourceDistanceModel: false,
        hrtf: hrtf || false,
  
        _err: 0,
        get err() {
          return this._err;
        },
        set err(val) {
          // Errors should not be overwritten by later errors until they are cleared by a query.
          if (this._err === 0 || val === 0) {
            this._err = val;
          }
        }
      };
      AL.deviceRefCounts[deviceId]++;
      AL.contexts[ctx.id] = ctx;
  
      if (hrtf !== null) {
        // Apply hrtf attrib to all contexts for this device
        for (var ctxId in AL.contexts) {
          var c = AL.contexts[ctxId];
          if (c.deviceId === deviceId) {
            c.hrtf = hrtf;
            AL.updateContextGlobal(c);
          }
        }
      }
  
      return ctx.id;
    };

  var _alcDestroyContext = (contextId) => {
      var ctx = AL.contexts[contextId];
      if (AL.currentCtx === ctx) {
        AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
        return;
      }
  
      // Stop playback, etc
      if (AL.contexts[contextId].interval) {
        clearInterval(AL.contexts[contextId].interval);
      }
      AL.deviceRefCounts[ctx.deviceId]--;
      delete AL.contexts[contextId];
      AL.freeIds.push(contextId);
    };

  var _alcGetContextsDevice = (contextId) => {
      if (contextId in AL.contexts) {
        return AL.contexts[contextId].deviceId;
      }
      return 0;
    };

  var _alcGetCurrentContext = () => {
      if (AL.currentCtx !== null) {
        return AL.currentCtx.id;
      }
      return 0;
    };

  var _alcGetIntegerv = (deviceId, param, size, pValues) => {
      if (size === 0 || !pValues) {
        // Ignore the query, per the spec
        return;
      }
  
      switch (param) {
      case 0x1000 /* ALC_MAJOR_VERSION */:
        HEAP32[((pValues)>>2)] = 1;
        break;
      case 0x1001 /* ALC_MINOR_VERSION */:
        HEAP32[((pValues)>>2)] = 1;
        break;
      case 0x1002 /* ALC_ATTRIBUTES_SIZE */:
        if (!(deviceId in AL.deviceRefCounts)) {
          AL.alcErr = 40961;
          return;
        }
        if (!AL.currentCtx) {
          AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
          return;
        }
  
        HEAP32[((pValues)>>2)] = AL.currentCtx.attrs.length;
        break;
      case 0x1003 /* ALC_ALL_ATTRIBUTES */:
        if (!(deviceId in AL.deviceRefCounts)) {
          AL.alcErr = 40961;
          return;
        }
        if (!AL.currentCtx) {
          AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
          return;
        }
  
        for (var i = 0; i < AL.currentCtx.attrs.length; i++) {
          HEAP32[(((pValues)+(i*4))>>2)] = AL.currentCtx.attrs[i];
        }
        break;
      case 0x1007 /* ALC_FREQUENCY */:
        if (!(deviceId in AL.deviceRefCounts)) {
          AL.alcErr = 40961;
          return;
        }
        if (!AL.currentCtx) {
          AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
          return;
        }
  
        HEAP32[((pValues)>>2)] = AL.currentCtx.audioCtx.sampleRate;
        break;
      case 0x1010 /* ALC_MONO_SOURCES */:
      case 0x1011 /* ALC_STEREO_SOURCES */:
        if (!(deviceId in AL.deviceRefCounts)) {
          AL.alcErr = 40961;
          return;
        }
        if (!AL.currentCtx) {
          AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
          return;
        }
  
        HEAP32[((pValues)>>2)] = 0x7FFFFFFF;
        break;
      case 0x1992 /* ALC_HRTF_SOFT */:
      case 0x1993 /* ALC_HRTF_STATUS_SOFT */:
        if (!(deviceId in AL.deviceRefCounts)) {
          AL.alcErr = 40961;
          return;
        }
  
        var hrtfStatus = 0 /* ALC_HRTF_DISABLED_SOFT */;
        for (var ctxId in AL.contexts) {
          var ctx = AL.contexts[ctxId];
          if (ctx.deviceId === deviceId) {
            hrtfStatus = ctx.hrtf ? 1 /* ALC_HRTF_ENABLED_SOFT */ : 0 /* ALC_HRTF_DISABLED_SOFT */;
          }
        }
        HEAP32[((pValues)>>2)] = hrtfStatus;
        break;
      case 0x1994 /* ALC_NUM_HRTF_SPECIFIERS_SOFT */:
        if (!(deviceId in AL.deviceRefCounts)) {
          AL.alcErr = 40961;
          return;
        }
        HEAP32[((pValues)>>2)] = 1;
        break;
      case 0x20003 /* ALC_MAX_AUXILIARY_SENDS */:
        if (!(deviceId in AL.deviceRefCounts)) {
          AL.alcErr = 40961;
          return;
        }
        if (!AL.currentCtx) {
          AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
          return;
        }
  
        HEAP32[((pValues)>>2)] = 1;
      case 0x312 /* ALC_CAPTURE_SAMPLES */:
        var c = AL.requireValidCaptureDevice(deviceId, 'alcGetIntegerv');
        if (!c) {
          return;
        }
        var n = c.capturedFrameCount;
        var dstfreq = c.requestedSampleRate;
        var srcfreq = c.audioCtx.sampleRate;
        var nsamples = Math.floor(n * (dstfreq/srcfreq));
        HEAP32[((pValues)>>2)] = nsamples;
        break;
      default:
        AL.alcErr = 40963;
        return;
      }
    };

  
  var _alcIsExtensionPresent = (deviceId, pExtName) => {
      var name = UTF8ToString(pExtName);
  
      return AL.ALC_EXTENSIONS[name] ? 1 : 0;
    };

  var _alcMakeContextCurrent = (contextId) => {
      if (contextId === 0) {
        AL.currentCtx = null;
      } else {
        AL.currentCtx = AL.contexts[contextId];
      }
      return 1;
    };

  
  var _alcOpenDevice = (pDeviceName) => {
      if (pDeviceName) {
        var name = UTF8ToString(pDeviceName);
        if (name !== AL.DEVICE_NAME) {
          return 0;
        }
      }
  
      if (typeof AudioContext != 'undefined' || typeof webkitAudioContext != 'undefined') {
        var deviceId = AL.newId();
        AL.deviceRefCounts[deviceId] = 0;
        return deviceId;
      }
      return 0;
    };

  
  var _emscripten_date_now = () => Date.now();
  
  var nowIsMonotonic = 1;
  
  var checkWasiClock = (clock_id) => clock_id >= 0 && clock_id <= 3;
  
  function _clock_time_get(clk_id, ignored_precision, ptime) {
    ignored_precision = bigintToI53Checked(ignored_precision);
  
  
      if (!checkWasiClock(clk_id)) {
        return 28;
      }
      var now;
      // all wasi clocks but realtime are monotonic
      if (clk_id === 0) {
        now = _emscripten_date_now();
      } else if (nowIsMonotonic) {
        now = _emscripten_get_now();
      } else {
        return 52;
      }
      // "now" is in ms, and wasi times are in ns.
      var nsec = Math.round(now * 1000 * 1000);
      HEAP64[((ptime)>>3)] = BigInt(nsec);
      return 0;
    ;
  }

  
  /** @param {number=} timeout */
  var safeSetTimeout = (func, timeout) => {
      
      return setTimeout(() => {
        
        callUserCallback(func);
      }, timeout);
    };
  
  
  
  var Browser = {
  useWebGL:false,
  isFullscreen:false,
  pointerLock:false,
  moduleContextCreatedCallbacks:[],
  workers:[],
  preloadedImages:{
  },
  preloadedAudios:{
  },
  getCanvas:() => Module['canvas'],
  init() {
        if (Browser.initted) return;
        Browser.initted = true;
  
        // Support for plugins that can process preloaded files. You can add more of these to
        // your app by creating and appending to preloadPlugins.
        //
        // Each plugin is asked if it can handle a file based on the file's name. If it can,
        // it is given the file's raw data. When it is done, it calls a callback with the file's
        // (possibly modified) data. For example, a plugin might decompress a file, or it
        // might create some side data structure for use later (like an Image element, etc.).
  
        var imagePlugin = {};
        imagePlugin['canHandle'] = function imagePlugin_canHandle(name) {
          return !Module['noImageDecoding'] && /\.(jpg|jpeg|png|bmp|webp)$/i.test(name);
        };
        imagePlugin['handle'] = function imagePlugin_handle(byteArray, name, onload, onerror) {
          var b = new Blob([byteArray], { type: Browser.getMimetype(name) });
          if (b.size !== byteArray.length) { // Safari bug #118630
            // Safari's Blob can only take an ArrayBuffer
            b = new Blob([(new Uint8Array(byteArray)).buffer], { type: Browser.getMimetype(name) });
          }
          var url = URL.createObjectURL(b);
          assert(typeof url == 'string', 'createObjectURL must return a url as a string');
          var img = new Image();
          img.onload = () => {
            assert(img.complete, `Image ${name} could not be decoded`);
            var canvas = /** @type {!HTMLCanvasElement} */ (document.createElement('canvas'));
            canvas.width = img.width;
            canvas.height = img.height;
            var ctx = canvas.getContext('2d');
            ctx.drawImage(img, 0, 0);
            Browser.preloadedImages[name] = canvas;
            URL.revokeObjectURL(url);
            onload?.(byteArray);
          };
          img.onerror = (event) => {
            err(`Image ${url} could not be decoded`);
            onerror?.();
          };
          img.src = url;
        };
        preloadPlugins.push(imagePlugin);
  
        var audioPlugin = {};
        audioPlugin['canHandle'] = function audioPlugin_canHandle(name) {
          return !Module['noAudioDecoding'] && name.slice(-4) in { '.ogg': 1, '.wav': 1, '.mp3': 1 };
        };
        audioPlugin['handle'] = function audioPlugin_handle(byteArray, name, onload, onerror) {
          var done = false;
          function finish(audio) {
            if (done) return;
            done = true;
            Browser.preloadedAudios[name] = audio;
            onload?.(byteArray);
          }
          function fail() {
            if (done) return;
            done = true;
            Browser.preloadedAudios[name] = new Audio(); // empty shim
            onerror?.();
          }
          var b = new Blob([byteArray], { type: Browser.getMimetype(name) });
          var url = URL.createObjectURL(b); // XXX we never revoke this!
          assert(typeof url == 'string', 'createObjectURL must return a url as a string');
          var audio = new Audio();
          audio.addEventListener('canplaythrough', () => finish(audio), false); // use addEventListener due to chromium bug 124926
          audio.onerror = function audio_onerror(event) {
            if (done) return;
            err(`warning: browser could not fully decode audio ${name}, trying slower base64 approach`);
            function encode64(data) {
              var BASE = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
              var PAD = '=';
              var ret = '';
              var leftchar = 0;
              var leftbits = 0;
              for (var i = 0; i < data.length; i++) {
                leftchar = (leftchar << 8) | data[i];
                leftbits += 8;
                while (leftbits >= 6) {
                  var curr = (leftchar >> (leftbits-6)) & 0x3f;
                  leftbits -= 6;
                  ret += BASE[curr];
                }
              }
              if (leftbits == 2) {
                ret += BASE[(leftchar&3) << 4];
                ret += PAD + PAD;
              } else if (leftbits == 4) {
                ret += BASE[(leftchar&0xf) << 2];
                ret += PAD;
              }
              return ret;
            }
            audio.src = 'data:audio/x-' + name.slice(-3) + ';base64,' + encode64(byteArray);
            finish(audio); // we don't wait for confirmation this worked - but it's worth trying
          };
          audio.src = url;
          // workaround for chrome bug 124926 - we do not always get oncanplaythrough or onerror
          safeSetTimeout(() => {
            finish(audio); // try to use it even though it is not necessarily ready to play
          }, 10000);
        };
        preloadPlugins.push(audioPlugin);
  
        // Canvas event setup
  
        function pointerLockChange() {
          var canvas = Browser.getCanvas();
          Browser.pointerLock = document['pointerLockElement'] === canvas ||
                                document['mozPointerLockElement'] === canvas ||
                                document['webkitPointerLockElement'] === canvas ||
                                document['msPointerLockElement'] === canvas;
        }
        var canvas = Browser.getCanvas();
        if (canvas) {
          // forced aspect ratio can be enabled by defining 'forcedAspectRatio' on Module
          // Module['forcedAspectRatio'] = 4 / 3;
  
          canvas.requestPointerLock = canvas['requestPointerLock'] ||
                                      canvas['mozRequestPointerLock'] ||
                                      canvas['webkitRequestPointerLock'] ||
                                      canvas['msRequestPointerLock'] ||
                                      (() => {});
          canvas.exitPointerLock = document['exitPointerLock'] ||
                                   document['mozExitPointerLock'] ||
                                   document['webkitExitPointerLock'] ||
                                   document['msExitPointerLock'] ||
                                   (() => {}); // no-op if function does not exist
          canvas.exitPointerLock = canvas.exitPointerLock.bind(document);
  
          document.addEventListener('pointerlockchange', pointerLockChange, false);
          document.addEventListener('mozpointerlockchange', pointerLockChange, false);
          document.addEventListener('webkitpointerlockchange', pointerLockChange, false);
          document.addEventListener('mspointerlockchange', pointerLockChange, false);
  
          if (Module['elementPointerLock']) {
            canvas.addEventListener("click", (ev) => {
              if (!Browser.pointerLock && Browser.getCanvas().requestPointerLock) {
                Browser.getCanvas().requestPointerLock();
                ev.preventDefault();
              }
            }, false);
          }
        }
      },
  createContext(/** @type {HTMLCanvasElement} */ canvas, useWebGL, setInModule, webGLContextAttributes) {
        if (useWebGL && Module['ctx'] && canvas == Browser.getCanvas()) return Module['ctx']; // no need to recreate GL context if it's already been created for this canvas.
  
        var ctx;
        var contextHandle;
        if (useWebGL) {
          // For GLES2/desktop GL compatibility, adjust a few defaults to be different to WebGL defaults, so that they align better with the desktop defaults.
          var contextAttributes = {
            antialias: false,
            alpha: false,
            majorVersion: 2,
          };
  
          if (webGLContextAttributes) {
            for (var attribute in webGLContextAttributes) {
              contextAttributes[attribute] = webGLContextAttributes[attribute];
            }
          }
  
          // This check of existence of GL is here to satisfy Closure compiler, which yells if variable GL is referenced below but GL object is not
          // actually compiled in because application is not doing any GL operations. TODO: Ideally if GL is not being used, this function
          // Browser.createContext() should not even be emitted.
          if (typeof GL != 'undefined') {
            contextHandle = GL.createContext(canvas, contextAttributes);
            if (contextHandle) {
              ctx = GL.getContext(contextHandle).GLctx;
            }
          }
        } else {
          ctx = canvas.getContext('2d');
        }
  
        if (!ctx) return null;
  
        if (setInModule) {
          if (!useWebGL) assert(typeof GLctx == 'undefined', 'cannot set in module if GLctx is used, but we are a non-GL context that would replace it');
          Module['ctx'] = ctx;
          if (useWebGL) GL.makeContextCurrent(contextHandle);
          Browser.useWebGL = useWebGL;
          Browser.moduleContextCreatedCallbacks.forEach((callback) => callback());
          Browser.init();
        }
        return ctx;
      },
  fullscreenHandlersInstalled:false,
  lockPointer:undefined,
  resizeCanvas:undefined,
  requestFullscreen(lockPointer, resizeCanvas) {
        Browser.lockPointer = lockPointer;
        Browser.resizeCanvas = resizeCanvas;
        if (typeof Browser.lockPointer == 'undefined') Browser.lockPointer = true;
        if (typeof Browser.resizeCanvas == 'undefined') Browser.resizeCanvas = false;
  
        var canvas = Browser.getCanvas();
        function fullscreenChange() {
          Browser.isFullscreen = false;
          var canvasContainer = canvas.parentNode;
          if ((document['fullscreenElement'] || document['mozFullScreenElement'] ||
               document['msFullscreenElement'] || document['webkitFullscreenElement'] ||
               document['webkitCurrentFullScreenElement']) === canvasContainer) {
            canvas.exitFullscreen = Browser.exitFullscreen;
            if (Browser.lockPointer) canvas.requestPointerLock();
            Browser.isFullscreen = true;
            if (Browser.resizeCanvas) {
              Browser.setFullscreenCanvasSize();
            } else {
              Browser.updateCanvasDimensions(canvas);
            }
          } else {
            // remove the full screen specific parent of the canvas again to restore the HTML structure from before going full screen
            canvasContainer.parentNode.insertBefore(canvas, canvasContainer);
            canvasContainer.parentNode.removeChild(canvasContainer);
  
            if (Browser.resizeCanvas) {
              Browser.setWindowedCanvasSize();
            } else {
              Browser.updateCanvasDimensions(canvas);
            }
          }
          Module['onFullScreen']?.(Browser.isFullscreen);
          Module['onFullscreen']?.(Browser.isFullscreen);
        }
  
        if (!Browser.fullscreenHandlersInstalled) {
          Browser.fullscreenHandlersInstalled = true;
          document.addEventListener('fullscreenchange', fullscreenChange, false);
          document.addEventListener('mozfullscreenchange', fullscreenChange, false);
          document.addEventListener('webkitfullscreenchange', fullscreenChange, false);
          document.addEventListener('MSFullscreenChange', fullscreenChange, false);
        }
  
        // create a new parent to ensure the canvas has no siblings. this allows browsers to optimize full screen performance when its parent is the full screen root
        var canvasContainer = document.createElement("div");
        canvas.parentNode.insertBefore(canvasContainer, canvas);
        canvasContainer.appendChild(canvas);
  
        // use parent of canvas as full screen root to allow aspect ratio correction (Firefox stretches the root to screen size)
        canvasContainer.requestFullscreen = canvasContainer['requestFullscreen'] ||
                                            canvasContainer['mozRequestFullScreen'] ||
                                            canvasContainer['msRequestFullscreen'] ||
                                           (canvasContainer['webkitRequestFullscreen'] ? () => canvasContainer['webkitRequestFullscreen'](Element['ALLOW_KEYBOARD_INPUT']) : null) ||
                                           (canvasContainer['webkitRequestFullScreen'] ? () => canvasContainer['webkitRequestFullScreen'](Element['ALLOW_KEYBOARD_INPUT']) : null);
  
        canvasContainer.requestFullscreen();
      },
  requestFullScreen() {
        abort('Module.requestFullScreen has been replaced by Module.requestFullscreen (without a capital S)');
      },
  exitFullscreen() {
        // This is workaround for chrome. Trying to exit from fullscreen
        // not in fullscreen state will cause "TypeError: Document not active"
        // in chrome. See https://github.com/emscripten-core/emscripten/pull/8236
        if (!Browser.isFullscreen) {
          return false;
        }
  
        var CFS = document['exitFullscreen'] ||
                  document['cancelFullScreen'] ||
                  document['mozCancelFullScreen'] ||
                  document['msExitFullscreen'] ||
                  document['webkitCancelFullScreen'] ||
            (() => {});
        CFS.apply(document, []);
        return true;
      },
  safeSetTimeout(func, timeout) {
        // Legacy function, this is used by the SDL2 port so we need to keep it
        // around at least until that is updated.
        // See https://github.com/libsdl-org/SDL/pull/6304
        return safeSetTimeout(func, timeout);
      },
  getMimetype(name) {
        return {
          'jpg': 'image/jpeg',
          'jpeg': 'image/jpeg',
          'png': 'image/png',
          'bmp': 'image/bmp',
          'ogg': 'audio/ogg',
          'wav': 'audio/wav',
          'mp3': 'audio/mpeg'
        }[name.slice(name.lastIndexOf('.')+1)];
      },
  getUserMedia(func) {
        window.getUserMedia ||= navigator['getUserMedia'] ||
                                navigator['mozGetUserMedia'];
        window.getUserMedia(func);
      },
  getMovementX(event) {
        return event['movementX'] ||
               event['mozMovementX'] ||
               event['webkitMovementX'] ||
               0;
      },
  getMovementY(event) {
        return event['movementY'] ||
               event['mozMovementY'] ||
               event['webkitMovementY'] ||
               0;
      },
  getMouseWheelDelta(event) {
        var delta = 0;
        switch (event.type) {
          case 'DOMMouseScroll':
            // 3 lines make up a step
            delta = event.detail / 3;
            break;
          case 'mousewheel':
            // 120 units make up a step
            delta = event.wheelDelta / 120;
            break;
          case 'wheel':
            delta = event.deltaY
            switch (event.deltaMode) {
              case 0:
                // DOM_DELTA_PIXEL: 100 pixels make up a step
                delta /= 100;
                break;
              case 1:
                // DOM_DELTA_LINE: 3 lines make up a step
                delta /= 3;
                break;
              case 2:
                // DOM_DELTA_PAGE: A page makes up 80 steps
                delta *= 80;
                break;
              default:
                throw 'unrecognized mouse wheel delta mode: ' + event.deltaMode;
            }
            break;
          default:
            throw 'unrecognized mouse wheel event: ' + event.type;
        }
        return delta;
      },
  mouseX:0,
  mouseY:0,
  mouseMovementX:0,
  mouseMovementY:0,
  touches:{
  },
  lastTouches:{
  },
  calculateMouseCoords(pageX, pageY) {
        // Calculate the movement based on the changes
        // in the coordinates.
        var canvas = Browser.getCanvas();
        var rect = canvas.getBoundingClientRect();
  
        // Neither .scrollX or .pageXOffset are defined in a spec, but
        // we prefer .scrollX because it is currently in a spec draft.
        // (see: http://www.w3.org/TR/2013/WD-cssom-view-20131217/)
        var scrollX = ((typeof window.scrollX != 'undefined') ? window.scrollX : window.pageXOffset);
        var scrollY = ((typeof window.scrollY != 'undefined') ? window.scrollY : window.pageYOffset);
        // If this assert lands, it's likely because the browser doesn't support scrollX or pageXOffset
        // and we have no viable fallback.
        assert((typeof scrollX != 'undefined') && (typeof scrollY != 'undefined'), 'Unable to retrieve scroll position, mouse positions likely broken.');
        var adjustedX = pageX - (scrollX + rect.left);
        var adjustedY = pageY - (scrollY + rect.top);
  
        // the canvas might be CSS-scaled compared to its backbuffer;
        // SDL-using content will want mouse coordinates in terms
        // of backbuffer units.
        adjustedX = adjustedX * (canvas.width / rect.width);
        adjustedY = adjustedY * (canvas.height / rect.height);
  
        return { x: adjustedX, y: adjustedY };
      },
  setMouseCoords(pageX, pageY) {
        const {x, y} = Browser.calculateMouseCoords(pageX, pageY);
        Browser.mouseMovementX = x - Browser.mouseX;
        Browser.mouseMovementY = y - Browser.mouseY;
        Browser.mouseX = x;
        Browser.mouseY = y;
      },
  calculateMouseEvent(event) { // event should be mousemove, mousedown or mouseup
        if (Browser.pointerLock) {
          // When the pointer is locked, calculate the coordinates
          // based on the movement of the mouse.
          // Workaround for Firefox bug 764498
          if (event.type != 'mousemove' &&
              ('mozMovementX' in event)) {
            Browser.mouseMovementX = Browser.mouseMovementY = 0;
          } else {
            Browser.mouseMovementX = Browser.getMovementX(event);
            Browser.mouseMovementY = Browser.getMovementY(event);
          }
  
          // add the mouse delta to the current absolute mouse position
          Browser.mouseX += Browser.mouseMovementX;
          Browser.mouseY += Browser.mouseMovementY;
        } else {
          if (event.type === 'touchstart' || event.type === 'touchend' || event.type === 'touchmove') {
            var touch = event.touch;
            if (touch === undefined) {
              return; // the "touch" property is only defined in SDL
  
            }
            var coords = Browser.calculateMouseCoords(touch.pageX, touch.pageY);
  
            if (event.type === 'touchstart') {
              Browser.lastTouches[touch.identifier] = coords;
              Browser.touches[touch.identifier] = coords;
            } else if (event.type === 'touchend' || event.type === 'touchmove') {
              var last = Browser.touches[touch.identifier];
              last ||= coords;
              Browser.lastTouches[touch.identifier] = last;
              Browser.touches[touch.identifier] = coords;
            }
            return;
          }
  
          Browser.setMouseCoords(event.pageX, event.pageY);
        }
      },
  resizeListeners:[],
  updateResizeListeners() {
        var canvas = Browser.getCanvas();
        Browser.resizeListeners.forEach((listener) => listener(canvas.width, canvas.height));
      },
  setCanvasSize(width, height, noUpdates) {
        var canvas = Browser.getCanvas();
        Browser.updateCanvasDimensions(canvas, width, height);
        if (!noUpdates) Browser.updateResizeListeners();
      },
  windowedWidth:0,
  windowedHeight:0,
  setFullscreenCanvasSize() {
        // check if SDL is available
        if (typeof SDL != "undefined") {
          var flags = HEAPU32[((SDL.screen)>>2)];
          flags = flags | 0x00800000; // set SDL_FULLSCREEN flag
          HEAP32[((SDL.screen)>>2)] = flags;
        }
        Browser.updateCanvasDimensions(Browser.getCanvas());
        Browser.updateResizeListeners();
      },
  setWindowedCanvasSize() {
        // check if SDL is available
        if (typeof SDL != "undefined") {
          var flags = HEAPU32[((SDL.screen)>>2)];
          flags = flags & ~0x00800000; // clear SDL_FULLSCREEN flag
          HEAP32[((SDL.screen)>>2)] = flags;
        }
        Browser.updateCanvasDimensions(Browser.getCanvas());
        Browser.updateResizeListeners();
      },
  updateCanvasDimensions(canvas, wNative, hNative) {
        if (wNative && hNative) {
          canvas.widthNative = wNative;
          canvas.heightNative = hNative;
        } else {
          wNative = canvas.widthNative;
          hNative = canvas.heightNative;
        }
        var w = wNative;
        var h = hNative;
        if (Module['forcedAspectRatio'] > 0) {
          if (w/h < Module['forcedAspectRatio']) {
            w = Math.round(h * Module['forcedAspectRatio']);
          } else {
            h = Math.round(w / Module['forcedAspectRatio']);
          }
        }
        if (((document['fullscreenElement'] || document['mozFullScreenElement'] ||
             document['msFullscreenElement'] || document['webkitFullscreenElement'] ||
             document['webkitCurrentFullScreenElement']) === canvas.parentNode) && (typeof screen != 'undefined')) {
           var factor = Math.min(screen.width / w, screen.height / h);
           w = Math.round(w * factor);
           h = Math.round(h * factor);
        }
        if (Browser.resizeCanvas) {
          if (canvas.width  != w) canvas.width  = w;
          if (canvas.height != h) canvas.height = h;
          if (typeof canvas.style != 'undefined') {
            canvas.style.removeProperty( "width");
            canvas.style.removeProperty("height");
          }
        } else {
          if (canvas.width  != wNative) canvas.width  = wNative;
          if (canvas.height != hNative) canvas.height = hNative;
          if (typeof canvas.style != 'undefined') {
            if (w != wNative || h != hNative) {
              canvas.style.setProperty( "width", w + "px", "important");
              canvas.style.setProperty("height", h + "px", "important");
            } else {
              canvas.style.removeProperty( "width");
              canvas.style.removeProperty("height");
            }
          }
        }
      },
  };
  
  var EGL = {
  errorCode:12288,
  defaultDisplayInitialized:false,
  currentContext:0,
  currentReadSurface:0,
  currentDrawSurface:0,
  contextAttributes:{
  alpha:false,
  depth:false,
  stencil:false,
  antialias:false,
  },
  stringCache:{
  },
  setErrorCode(code) {
        EGL.errorCode = code;
      },
  chooseConfig(display, attribList, config, config_size, numConfigs) {
        if (display != 62000) {
          EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
          return 0;
        }
  
        if (attribList) {
          // read attribList if it is non-null
          for (;;) {
            var param = HEAP32[((attribList)>>2)];
            if (param == 0x3021 /*EGL_ALPHA_SIZE*/) {
              var alphaSize = HEAP32[(((attribList)+(4))>>2)];
              EGL.contextAttributes.alpha = (alphaSize > 0);
            } else if (param == 0x3025 /*EGL_DEPTH_SIZE*/) {
              var depthSize = HEAP32[(((attribList)+(4))>>2)];
              EGL.contextAttributes.depth = (depthSize > 0);
            } else if (param == 0x3026 /*EGL_STENCIL_SIZE*/) {
              var stencilSize = HEAP32[(((attribList)+(4))>>2)];
              EGL.contextAttributes.stencil = (stencilSize > 0);
            } else if (param == 0x3031 /*EGL_SAMPLES*/) {
              var samples = HEAP32[(((attribList)+(4))>>2)];
              EGL.contextAttributes.antialias = (samples > 0);
            } else if (param == 0x3032 /*EGL_SAMPLE_BUFFERS*/) {
              var samples = HEAP32[(((attribList)+(4))>>2)];
              EGL.contextAttributes.antialias = (samples == 1);
            } else if (param == 0x3100 /*EGL_CONTEXT_PRIORITY_LEVEL_IMG*/) {
              var requestedPriority = HEAP32[(((attribList)+(4))>>2)];
              EGL.contextAttributes.lowLatency = (requestedPriority != 0x3103 /*EGL_CONTEXT_PRIORITY_LOW_IMG*/);
            } else if (param == 0x3038 /*EGL_NONE*/) {
                break;
            }
            attribList += 8;
          }
        }
  
        if ((!config || !config_size) && !numConfigs) {
          EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
          return 0;
        }
        if (numConfigs) {
          HEAP32[((numConfigs)>>2)] = 1; // Total number of supported configs: 1.
        }
        if (config && config_size > 0) {
          HEAPU32[((config)>>2)] = 62002;
        }
  
        EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
        return 1;
      },
  };
  var _eglBindAPI = (api) => {
      if (api == 0x30A0 /* EGL_OPENGL_ES_API */) {
        EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
        return 1;
      }
      // if (api == 0x30A1 /* EGL_OPENVG_API */ || api == 0x30A2 /* EGL_OPENGL_API */) {
      EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
      return 0;
    };

  var _eglChooseConfig = (display, attrib_list, configs, config_size, numConfigs) =>
      EGL.chooseConfig(display, attrib_list, configs, config_size, numConfigs);

  var GLctx;
  
  var webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance = (ctx) =>
      // Closure is expected to be allowed to minify the '.dibvbi' property, so not accessing it quoted.
      !!(ctx.dibvbi = ctx.getExtension('WEBGL_draw_instanced_base_vertex_base_instance'));
  
  var webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance = (ctx) => {
      // Closure is expected to be allowed to minify the '.mdibvbi' property, so not accessing it quoted.
      return !!(ctx.mdibvbi = ctx.getExtension('WEBGL_multi_draw_instanced_base_vertex_base_instance'));
    };
  
  var webgl_enable_EXT_polygon_offset_clamp = (ctx) =>
      !!(ctx.extPolygonOffsetClamp = ctx.getExtension('EXT_polygon_offset_clamp'));
  
  var webgl_enable_EXT_clip_control = (ctx) =>
      !!(ctx.extClipControl = ctx.getExtension('EXT_clip_control'));
  
  var webgl_enable_WEBGL_polygon_mode = (ctx) =>
      !!(ctx.webglPolygonMode = ctx.getExtension('WEBGL_polygon_mode'));
  
  var webgl_enable_WEBGL_multi_draw = (ctx) =>
      // Closure is expected to be allowed to minify the '.multiDrawWebgl' property, so not accessing it quoted.
      !!(ctx.multiDrawWebgl = ctx.getExtension('WEBGL_multi_draw'));
  
  var getEmscriptenSupportedExtensions = (ctx) => {
      // Restrict the list of advertised extensions to those that we actually
      // support.
      var supportedExtensions = [
        // WebGL 2 extensions
        'EXT_color_buffer_float',
        'EXT_conservative_depth',
        'EXT_disjoint_timer_query_webgl2',
        'EXT_texture_norm16',
        'NV_shader_noperspective_interpolation',
        'WEBGL_clip_cull_distance',
        // WebGL 1 and WebGL 2 extensions
        'EXT_clip_control',
        'EXT_color_buffer_half_float',
        'EXT_depth_clamp',
        'EXT_float_blend',
        'EXT_polygon_offset_clamp',
        'EXT_texture_compression_bptc',
        'EXT_texture_compression_rgtc',
        'EXT_texture_filter_anisotropic',
        'KHR_parallel_shader_compile',
        'OES_texture_float_linear',
        'WEBGL_blend_func_extended',
        'WEBGL_compressed_texture_astc',
        'WEBGL_compressed_texture_etc',
        'WEBGL_compressed_texture_etc1',
        'WEBGL_compressed_texture_s3tc',
        'WEBGL_compressed_texture_s3tc_srgb',
        'WEBGL_debug_renderer_info',
        'WEBGL_debug_shaders',
        'WEBGL_lose_context',
        'WEBGL_multi_draw',
        'WEBGL_polygon_mode'
      ];
      // .getSupportedExtensions() can return null if context is lost, so coerce to empty array.
      return (ctx.getSupportedExtensions() || []).filter(ext => supportedExtensions.includes(ext));
    };
  
  
  var GL = {
  counter:1,
  buffers:[],
  programs:[],
  framebuffers:[],
  renderbuffers:[],
  textures:[],
  shaders:[],
  vaos:[],
  contexts:[],
  offscreenCanvases:{
  },
  queries:[],
  samplers:[],
  transformFeedbacks:[],
  syncs:[],
  stringCache:{
  },
  stringiCache:{
  },
  unpackAlignment:4,
  unpackRowLength:0,
  recordError:(errorCode) => {
        if (!GL.lastError) {
          GL.lastError = errorCode;
        }
      },
  getNewId:(table) => {
        var ret = GL.counter++;
        for (var i = table.length; i < ret; i++) {
          table[i] = null;
        }
        return ret;
      },
  genObject:(n, buffers, createFunction, objectTable
        ) => {
        for (var i = 0; i < n; i++) {
          var buffer = GLctx[createFunction]();
          var id = buffer && GL.getNewId(objectTable);
          if (buffer) {
            buffer.name = id;
            objectTable[id] = buffer;
          } else {
            GL.recordError(0x502 /* GL_INVALID_OPERATION */);
          }
          HEAP32[(((buffers)+(i*4))>>2)] = id;
        }
      },
  getSource:(shader, count, string, length) => {
        var source = '';
        for (var i = 0; i < count; ++i) {
          var len = length ? HEAPU32[(((length)+(i*4))>>2)] : undefined;
          source += UTF8ToString(HEAPU32[(((string)+(i*4))>>2)], len);
        }
        return source;
      },
  createContext:(/** @type {HTMLCanvasElement} */ canvas, webGLContextAttributes) => {
  
        // BUG: Workaround Safari WebGL issue: After successfully acquiring WebGL
        // context on a canvas, calling .getContext() will always return that
        // context independent of which 'webgl' or 'webgl2'
        // context version was passed. See:
        //   https://bugs.webkit.org/show_bug.cgi?id=222758
        // and:
        //   https://github.com/emscripten-core/emscripten/issues/13295.
        // TODO: Once the bug is fixed and shipped in Safari, adjust the Safari
        // version field in above check.
        if (!canvas.getContextSafariWebGL2Fixed) {
          canvas.getContextSafariWebGL2Fixed = canvas.getContext;
          /** @type {function(this:HTMLCanvasElement, string, (Object|null)=): (Object|null)} */
          function fixedGetContext(ver, attrs) {
            var gl = canvas.getContextSafariWebGL2Fixed(ver, attrs);
            return ((ver == 'webgl') == (gl instanceof WebGLRenderingContext)) ? gl : null;
          }
          canvas.getContext = fixedGetContext;
        }
  
        var ctx = canvas.getContext("webgl2", webGLContextAttributes);
  
        if (!ctx) return 0;
  
        var handle = GL.registerContext(ctx, webGLContextAttributes);
  
        return handle;
      },
  registerContext:(ctx, webGLContextAttributes) => {
        // without pthreads a context is just an integer ID
        var handle = GL.getNewId(GL.contexts);
  
        var context = {
          handle,
          attributes: webGLContextAttributes,
          version: webGLContextAttributes.majorVersion,
          GLctx: ctx
        };
  
        // Store the created context object so that we can access the context
        // given a canvas without having to pass the parameters again.
        if (ctx.canvas) ctx.canvas.GLctxObject = context;
        GL.contexts[handle] = context;
        if (typeof webGLContextAttributes.enableExtensionsByDefault == 'undefined' || webGLContextAttributes.enableExtensionsByDefault) {
          GL.initExtensions(context);
        }
  
        return handle;
      },
  makeContextCurrent:(contextHandle) => {
  
        // Active Emscripten GL layer context object.
        GL.currentContext = GL.contexts[contextHandle];
        // Active WebGL context object.
        Module['ctx'] = GLctx = GL.currentContext?.GLctx;
        return !(contextHandle && !GLctx);
      },
  getContext:(contextHandle) => {
        return GL.contexts[contextHandle];
      },
  deleteContext:(contextHandle) => {
        if (GL.currentContext === GL.contexts[contextHandle]) {
          GL.currentContext = null;
        }
        if (typeof JSEvents == 'object') {
          // Release all JS event handlers on the DOM element that the GL context is
          // associated with since the context is now deleted.
          JSEvents.removeAllHandlersOnTarget(GL.contexts[contextHandle].GLctx.canvas);
        }
        // Make sure the canvas object no longer refers to the context object so
        // there are no GC surprises.
        if (GL.contexts[contextHandle]?.GLctx.canvas) {
          GL.contexts[contextHandle].GLctx.canvas.GLctxObject = undefined;
        }
        GL.contexts[contextHandle] = null;
      },
  initExtensions:(context) => {
        // If this function is called without a specific context object, init the
        // extensions of the currently active context.
        context ||= GL.currentContext;
  
        if (context.initExtensionsDone) return;
        context.initExtensionsDone = true;
  
        var GLctx = context.GLctx;
  
        // Detect the presence of a few extensions manually, ction GL interop
        // layer itself will need to know if they exist.
  
        // Extensions that are available in both WebGL 1 and WebGL 2
        webgl_enable_WEBGL_multi_draw(GLctx);
        webgl_enable_EXT_polygon_offset_clamp(GLctx);
        webgl_enable_EXT_clip_control(GLctx);
        webgl_enable_WEBGL_polygon_mode(GLctx);
        // Extensions that are available from WebGL >= 2 (no-op if called on a WebGL 1 context active)
        webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(GLctx);
        webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(GLctx);
  
        // On WebGL 2, EXT_disjoint_timer_query is replaced with an alternative
        // that's based on core APIs, and exposes only the queryCounterEXT()
        // entrypoint.
        if (context.version >= 2) {
          GLctx.disjointTimerQueryExt = GLctx.getExtension("EXT_disjoint_timer_query_webgl2");
        }
  
        // However, Firefox exposes the WebGL 1 version on WebGL 2 as well and
        // thus we look for the WebGL 1 version again if the WebGL 2 version
        // isn't present. https://bugzilla.mozilla.org/show_bug.cgi?id=1328882
        if (context.version < 2 || !GLctx.disjointTimerQueryExt)
        {
          GLctx.disjointTimerQueryExt = GLctx.getExtension("EXT_disjoint_timer_query");
        }
  
        getEmscriptenSupportedExtensions(GLctx).forEach((ext) => {
          // WEBGL_lose_context, WEBGL_debug_renderer_info and WEBGL_debug_shaders
          // are not enabled by default.
          if (!ext.includes('lose_context') && !ext.includes('debug')) {
            // Call .getExtension() to enable that extension permanently.
            GLctx.getExtension(ext);
          }
        });
      },
  };
  
  var _eglCreateContext = (display, config, hmm, contextAttribs) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
  
      // EGL 1.4 spec says default EGL_CONTEXT_CLIENT_VERSION is GLES1, but this is not supported by Emscripten.
      // So user must pass EGL_CONTEXT_CLIENT_VERSION == 2 to initialize EGL.
      var glesContextVersion = 1;
      for (;;) {
        var param = HEAP32[((contextAttribs)>>2)];
        if (param == 0x3098 /*EGL_CONTEXT_CLIENT_VERSION*/) {
          glesContextVersion = HEAP32[(((contextAttribs)+(4))>>2)];
        } else if (param == 0x3038 /*EGL_NONE*/) {
          break;
        } else {
          /* EGL1.4 specifies only EGL_CONTEXT_CLIENT_VERSION as supported attribute */
          EGL.setErrorCode(0x3004 /*EGL_BAD_ATTRIBUTE*/);
          return 0;
        }
        contextAttribs += 8;
      }
      if (glesContextVersion < 2 || glesContextVersion > 3) {
        EGL.setErrorCode(0x3005 /* EGL_BAD_CONFIG */);
        return 0; /* EGL_NO_CONTEXT */
      }
  
      EGL.contextAttributes.majorVersion = glesContextVersion - 1; // WebGL 1 is GLES 2, WebGL2 is GLES3
      EGL.contextAttributes.minorVersion = 0;
  
      EGL.context = GL.createContext(Browser.getCanvas(), EGL.contextAttributes);
  
      if (EGL.context != 0) {
        EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
  
        // Run callbacks so that GL emulation works
        GL.makeContextCurrent(EGL.context);
        Browser.useWebGL = true;
        Browser.moduleContextCreatedCallbacks.forEach((callback) => callback());
  
        // Note: This function only creates a context, but it shall not make it active.
        GL.makeContextCurrent(null);
        return 62004;
      } else {
        EGL.setErrorCode(0x3009 /* EGL_BAD_MATCH */); // By the EGL 1.4 spec, an implementation that does not support GLES2 (WebGL in this case), this error code is set.
        return 0; /* EGL_NO_CONTEXT */
      }
    };

  var _eglCreateWindowSurface = (display, config, win, attrib_list) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      if (config != 62002) {
        EGL.setErrorCode(0x3005 /* EGL_BAD_CONFIG */);
        return 0;
      }
      // TODO: Examine attrib_list! Parameters that can be present there are:
      // - EGL_RENDER_BUFFER (must be EGL_BACK_BUFFER)
      // - EGL_VG_COLORSPACE (can't be set)
      // - EGL_VG_ALPHA_FORMAT (can't be set)
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 62006; /* Magic ID for Emscripten 'default surface' */
    };

  
  var _eglDestroyContext = (display, context) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      if (context != 62004) {
        EGL.setErrorCode(0x3006 /* EGL_BAD_CONTEXT */);
        return 0;
      }
  
      GL.deleteContext(EGL.context);
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      if (EGL.currentContext == context) {
        EGL.currentContext = 0;
      }
      return 1 /* EGL_TRUE */;
    };

  var _eglDestroySurface = (display, surface) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      if (surface != 62006 /* Magic ID for the only EGLSurface supported by Emscripten */) {
        EGL.setErrorCode(0x300D /* EGL_BAD_SURFACE */);
        return 1;
      }
      if (EGL.currentReadSurface == surface) {
        EGL.currentReadSurface = 0;
      }
      if (EGL.currentDrawSurface == surface) {
        EGL.currentDrawSurface = 0;
      }
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1; /* Magic ID for Emscripten 'default surface' */
    };

  var _eglGetConfigAttrib = (display, config, attribute, value) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      if (config != 62002) {
        EGL.setErrorCode(0x3005 /* EGL_BAD_CONFIG */);
        return 0;
      }
      if (!value) {
        EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
        return 0;
      }
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      switch (attribute) {
      case 0x3020: // EGL_BUFFER_SIZE
        HEAP32[((value)>>2)] = EGL.contextAttributes.alpha ? 32 : 24;
        return 1;
      case 0x3021: // EGL_ALPHA_SIZE
        HEAP32[((value)>>2)] = EGL.contextAttributes.alpha ? 8 : 0;
        return 1;
      case 0x3022: // EGL_BLUE_SIZE
        HEAP32[((value)>>2)] = 8;
        return 1;
      case 0x3023: // EGL_GREEN_SIZE
        HEAP32[((value)>>2)] = 8;
        return 1;
      case 0x3024: // EGL_RED_SIZE
        HEAP32[((value)>>2)] = 8;
        return 1;
      case 0x3025: // EGL_DEPTH_SIZE
        HEAP32[((value)>>2)] = EGL.contextAttributes.depth ? 24 : 0;
        return 1;
      case 0x3026: // EGL_STENCIL_SIZE
        HEAP32[((value)>>2)] = EGL.contextAttributes.stencil ? 8 : 0;
        return 1;
      case 0x3027: // EGL_CONFIG_CAVEAT
        // We can return here one of EGL_NONE (0x3038), EGL_SLOW_CONFIG (0x3050) or EGL_NON_CONFORMANT_CONFIG (0x3051).
        HEAP32[((value)>>2)] = 0x3038;
        return 1;
      case 0x3028: // EGL_CONFIG_ID
        HEAP32[((value)>>2)] = 62002;
        return 1;
      case 0x3029: // EGL_LEVEL
        HEAP32[((value)>>2)] = 0;
        return 1;
      case 0x302A: // EGL_MAX_PBUFFER_HEIGHT
        HEAP32[((value)>>2)] = 4096;
        return 1;
      case 0x302B: // EGL_MAX_PBUFFER_PIXELS
        HEAP32[((value)>>2)] = 16777216;
        return 1;
      case 0x302C: // EGL_MAX_PBUFFER_WIDTH
        HEAP32[((value)>>2)] = 4096;
        return 1;
      case 0x302D: // EGL_NATIVE_RENDERABLE
        HEAP32[((value)>>2)] = 0;
        return 1;
      case 0x302E: // EGL_NATIVE_VISUAL_ID
        HEAP32[((value)>>2)] = 0;
        return 1;
      case 0x302F: // EGL_NATIVE_VISUAL_TYPE
        HEAP32[((value)>>2)] = 0x3038;
        return 1;
      case 0x3031: // EGL_SAMPLES
        HEAP32[((value)>>2)] = EGL.contextAttributes.antialias ? 4 : 0;
        return 1;
      case 0x3032: // EGL_SAMPLE_BUFFERS
        HEAP32[((value)>>2)] = EGL.contextAttributes.antialias ? 1 : 0;
        return 1;
      case 0x3033: // EGL_SURFACE_TYPE
        HEAP32[((value)>>2)] = 0x4;
        return 1;
      case 0x3034: // EGL_TRANSPARENT_TYPE
        // If this returns EGL_TRANSPARENT_RGB (0x3052), transparency is used through color-keying. No such thing applies to Emscripten canvas.
        HEAP32[((value)>>2)] = 0x3038;
        return 1;
      case 0x3035: // EGL_TRANSPARENT_BLUE_VALUE
      case 0x3036: // EGL_TRANSPARENT_GREEN_VALUE
      case 0x3037: // EGL_TRANSPARENT_RED_VALUE
        // "If EGL_TRANSPARENT_TYPE is EGL_NONE, then the values for EGL_TRANSPARENT_RED_VALUE, EGL_TRANSPARENT_GREEN_VALUE, and EGL_TRANSPARENT_BLUE_VALUE are undefined."
        HEAP32[((value)>>2)] = -1;
        return 1;
      case 0x3039: // EGL_BIND_TO_TEXTURE_RGB
      case 0x303A: // EGL_BIND_TO_TEXTURE_RGBA
        HEAP32[((value)>>2)] = 0;
        return 1;
      case 0x303B: // EGL_MIN_SWAP_INTERVAL
        HEAP32[((value)>>2)] = 0;
        return 1;
      case 0x303C: // EGL_MAX_SWAP_INTERVAL
        HEAP32[((value)>>2)] = 1;
        return 1;
      case 0x303D: // EGL_LUMINANCE_SIZE
      case 0x303E: // EGL_ALPHA_MASK_SIZE
        HEAP32[((value)>>2)] = 0;
        return 1;
      case 0x303F: // EGL_COLOR_BUFFER_TYPE
        // EGL has two types of buffers: EGL_RGB_BUFFER and EGL_LUMINANCE_BUFFER.
        HEAP32[((value)>>2)] = 0x308E;
        return 1;
      case 0x3040: // EGL_RENDERABLE_TYPE
        // A bit combination of EGL_OPENGL_ES_BIT,EGL_OPENVG_BIT,EGL_OPENGL_ES2_BIT and EGL_OPENGL_BIT.
        HEAP32[((value)>>2)] = 0x4;
        return 1;
      case 0x3042: // EGL_CONFORMANT
        // "EGL_CONFORMANT is a mask indicating if a client API context created with respect to the corresponding EGLConfig will pass the required conformance tests for that API."
        HEAP32[((value)>>2)] = 0;
        return 1;
      default:
        EGL.setErrorCode(0x3004 /* EGL_BAD_ATTRIBUTE */);
        return 0;
      }
    };

  var _eglGetDisplay = (nativeDisplayType) => {
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      // Emscripten EGL implementation "emulates" X11, and eglGetDisplay is
      // expected to accept/receive a pointer to an X11 Display object (or
      // EGL_DEFAULT_DISPLAY).
      if (nativeDisplayType != 0 /* EGL_DEFAULT_DISPLAY */ && nativeDisplayType != 1 /* see library_xlib.js */) {
        return 0; // EGL_NO_DISPLAY
      }
      return 62000;
    };

  var _eglGetError = () => EGL.errorCode;

  var _eglInitialize = (display, majorVersion, minorVersion) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      if (majorVersion) {
        HEAP32[((majorVersion)>>2)] = 1; // Advertise EGL Major version: '1'
      }
      if (minorVersion) {
        HEAP32[((minorVersion)>>2)] = 4; // Advertise EGL Minor version: '4'
      }
      EGL.defaultDisplayInitialized = true;
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1;
    };

  
  var _eglMakeCurrent = (display, draw, read, context) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0 /* EGL_FALSE */;
      }
      //\todo An EGL_NOT_INITIALIZED error is generated if EGL is not initialized for dpy.
      if (context != 0 && context != 62004) {
        EGL.setErrorCode(0x3006 /* EGL_BAD_CONTEXT */);
        return 0;
      }
      if ((read != 0 && read != 62006) || (draw != 0 && draw != 62006 /* Magic ID for Emscripten 'default surface' */)) {
        EGL.setErrorCode(0x300D /* EGL_BAD_SURFACE */);
        return 0;
      }
  
      GL.makeContextCurrent(context ? EGL.context : null);
  
      EGL.currentContext = context;
      EGL.currentDrawSurface = draw;
      EGL.currentReadSurface = read;
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1 /* EGL_TRUE */;
    };

  
  var _eglQueryString = (display, name) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      //\todo An EGL_NOT_INITIALIZED error is generated if EGL is not initialized for dpy.
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      if (EGL.stringCache[name]) return EGL.stringCache[name];
      var ret;
      switch (name) {
        case 0x3053 /* EGL_VENDOR */: ret = stringToNewUTF8("Emscripten"); break;
        case 0x3054 /* EGL_VERSION */: ret = stringToNewUTF8("1.4 Emscripten EGL"); break;
        case 0x3055 /* EGL_EXTENSIONS */:  ret = stringToNewUTF8(""); break; // Currently not supporting any EGL extensions.
        case 0x308D /* EGL_CLIENT_APIS */: ret = stringToNewUTF8("OpenGL_ES"); break;
        default:
          EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
          return 0;
      }
      EGL.stringCache[name] = ret;
      return ret;
    };

  
  var _eglSwapBuffers = (dpy, surface) => {
  
      if (!EGL.defaultDisplayInitialized) {
        EGL.setErrorCode(0x3001 /* EGL_NOT_INITIALIZED */);
      } else if (!GLctx) {
        EGL.setErrorCode(0x3002 /* EGL_BAD_ACCESS */);
      } else if (GLctx.isContextLost()) {
        EGL.setErrorCode(0x300E /* EGL_CONTEXT_LOST */);
      } else {
        // According to documentation this does an implicit flush.
        // Due to discussion at https://github.com/emscripten-core/emscripten/pull/1871
        // the flush was removed since this _may_ result in slowing code down.
        //_glFlush();
        EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
        return 1 /* EGL_TRUE */;
      }
      return 0 /* EGL_FALSE */;
    };

  
  var _eglSwapInterval = (display, interval) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      if (interval == 0) _emscripten_set_main_loop_timing(0, 0);
      else _emscripten_set_main_loop_timing(1, interval);
  
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1;
    };

  var _eglTerminate = (display) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      EGL.currentContext = 0;
      EGL.currentReadSurface = 0;
      EGL.currentDrawSurface = 0;
      EGL.defaultDisplayInitialized = false;
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1;
    };

  
  /** @suppress {duplicate } */
  var _eglWaitClient = () => {
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1;
    };
  var _eglWaitGL = _eglWaitClient;

  var _eglWaitNative = (nativeEngineId) => {
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1;
    };

  var readEmAsmArgsArray = [];
  var readEmAsmArgs = (sigPtr, buf) => {
      // Nobody should have mutated _readEmAsmArgsArray underneath us to be something else than an array.
      assert(Array.isArray(readEmAsmArgsArray));
      // The input buffer is allocated on the stack, so it must be stack-aligned.
      assert(buf % 16 == 0);
      readEmAsmArgsArray.length = 0;
      var ch;
      // Most arguments are i32s, so shift the buffer pointer so it is a plain
      // index into HEAP32.
      while (ch = HEAPU8[sigPtr++]) {
        var chr = String.fromCharCode(ch);
        var validChars = ['d', 'f', 'i', 'p'];
        // In WASM_BIGINT mode we support passing i64 values as bigint.
        validChars.push('j');
        assert(validChars.includes(chr), `Invalid character ${ch}("${chr}") in readEmAsmArgs! Use only [${validChars}], and do not specify "v" for void return argument.`);
        // Floats are always passed as doubles, so all types except for 'i'
        // are 8 bytes and require alignment.
        var wide = (ch != 105);
        wide &= (ch != 112);
        buf += wide && (buf % 8) ? 4 : 0;
        readEmAsmArgsArray.push(
          // Special case for pointers under wasm64 or CAN_ADDRESS_2GB mode.
          ch == 112 ? HEAPU32[((buf)>>2)] :
          ch == 106 ? HEAP64[((buf)>>3)] :
          ch == 105 ?
            HEAP32[((buf)>>2)] :
            HEAPF64[((buf)>>3)]
        );
        buf += wide ? 8 : 4;
      }
      return readEmAsmArgsArray;
    };
  var runEmAsmFunction = (code, sigPtr, argbuf) => {
      var args = readEmAsmArgs(sigPtr, argbuf);
      assert(ASM_CONSTS.hasOwnProperty(code), `No EM_ASM constant found at address ${code}.  The loaded WebAssembly file is likely out of sync with the generated JavaScript.`);
      return ASM_CONSTS[code](...args);
    };
  var _emscripten_asm_const_int = (code, sigPtr, argbuf) => {
      return runEmAsmFunction(code, sigPtr, argbuf);
    };

  var runMainThreadEmAsm = (emAsmAddr, sigPtr, argbuf, sync) => {
      var args = readEmAsmArgs(sigPtr, argbuf);
      assert(ASM_CONSTS.hasOwnProperty(emAsmAddr), `No EM_ASM constant found at address ${emAsmAddr}.  The loaded WebAssembly file is likely out of sync with the generated JavaScript.`);
      return ASM_CONSTS[emAsmAddr](...args);
    };
  var _emscripten_asm_const_int_sync_on_main_thread = (emAsmAddr, sigPtr, argbuf) => runMainThreadEmAsm(emAsmAddr, sigPtr, argbuf, 1);

  var _emscripten_asm_const_ptr_sync_on_main_thread = (emAsmAddr, sigPtr, argbuf) => runMainThreadEmAsm(emAsmAddr, sigPtr, argbuf, 1);


  var _emscripten_err = (str) => err(UTF8ToString(str));

  var onExits = [];
  var addOnExit = (cb) => onExits.push(cb);
  var JSEvents = {
  memcpy(target, src, size) {
        HEAP8.set(HEAP8.subarray(src, src + size), target);
      },
  removeAllEventListeners() {
        while (JSEvents.eventHandlers.length) {
          JSEvents._removeHandler(JSEvents.eventHandlers.length - 1);
        }
        JSEvents.deferredCalls = [];
      },
  inEventHandler:0,
  deferredCalls:[],
  deferCall(targetFunction, precedence, argsList) {
        function arraysHaveEqualContent(arrA, arrB) {
          if (arrA.length != arrB.length) return false;
  
          for (var i in arrA) {
            if (arrA[i] != arrB[i]) return false;
          }
          return true;
        }
        // Test if the given call was already queued, and if so, don't add it again.
        for (var call of JSEvents.deferredCalls) {
          if (call.targetFunction == targetFunction && arraysHaveEqualContent(call.argsList, argsList)) {
            return;
          }
        }
        JSEvents.deferredCalls.push({
          targetFunction,
          precedence,
          argsList
        });
  
        JSEvents.deferredCalls.sort((x,y) => x.precedence < y.precedence);
      },
  removeDeferredCalls(targetFunction) {
        JSEvents.deferredCalls = JSEvents.deferredCalls.filter((call) => call.targetFunction != targetFunction);
      },
  canPerformEventHandlerRequests() {
        if (navigator.userActivation) {
          // Verify against transient activation status from UserActivation API
          // whether it is possible to perform a request here without needing to defer. See
          // https://developer.mozilla.org/en-US/docs/Web/Security/User_activation#transient_activation
          // and https://caniuse.com/mdn-api_useractivation
          // At the time of writing, Firefox does not support this API: https://bugzilla.mozilla.org/show_bug.cgi?id=1791079
          return navigator.userActivation.isActive;
        }
  
        return JSEvents.inEventHandler && JSEvents.currentEventHandler.allowsDeferredCalls;
      },
  runDeferredCalls() {
        if (!JSEvents.canPerformEventHandlerRequests()) {
          return;
        }
        var deferredCalls = JSEvents.deferredCalls;
        JSEvents.deferredCalls = [];
        for (var call of deferredCalls) {
          call.targetFunction(...call.argsList);
        }
      },
  eventHandlers:[],
  removeAllHandlersOnTarget:(target, eventTypeString) => {
        for (var i = 0; i < JSEvents.eventHandlers.length; ++i) {
          if (JSEvents.eventHandlers[i].target == target &&
            (!eventTypeString || eventTypeString == JSEvents.eventHandlers[i].eventTypeString)) {
             JSEvents._removeHandler(i--);
           }
        }
      },
  _removeHandler(i) {
        var h = JSEvents.eventHandlers[i];
        h.target.removeEventListener(h.eventTypeString, h.eventListenerFunc, h.useCapture);
        JSEvents.eventHandlers.splice(i, 1);
      },
  registerOrRemoveHandler(eventHandler) {
        if (!eventHandler.target) {
          err('registerOrRemoveHandler: the target element for event handler registration does not exist, when processing the following event handler registration:');
          console.dir(eventHandler);
          return -4;
        }
        if (eventHandler.callbackfunc) {
          eventHandler.eventListenerFunc = function(event) {
            // Increment nesting count for the event handler.
            ++JSEvents.inEventHandler;
            JSEvents.currentEventHandler = eventHandler;
            // Process any old deferred calls the user has placed.
            JSEvents.runDeferredCalls();
            // Process the actual event, calls back to user C code handler.
            eventHandler.handlerFunc(event);
            // Process any new deferred calls that were placed right now from this event handler.
            JSEvents.runDeferredCalls();
            // Out of event handler - restore nesting count.
            --JSEvents.inEventHandler;
          };
  
          eventHandler.target.addEventListener(eventHandler.eventTypeString,
                                               eventHandler.eventListenerFunc,
                                               eventHandler.useCapture);
          JSEvents.eventHandlers.push(eventHandler);
        } else {
          for (var i = 0; i < JSEvents.eventHandlers.length; ++i) {
            if (JSEvents.eventHandlers[i].target == eventHandler.target
             && JSEvents.eventHandlers[i].eventTypeString == eventHandler.eventTypeString) {
               JSEvents._removeHandler(i--);
             }
          }
        }
        return 0;
      },
  getNodeNameForTarget(target) {
        if (!target) return '';
        if (target == window) return '#window';
        if (target == screen) return '#screen';
        return target?.nodeName || '';
      },
  fullscreenEnabled() {
        return document.fullscreenEnabled
        // Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitFullscreenEnabled.
        // TODO: If Safari at some point ships with unprefixed version, update the version check above.
        || document.webkitFullscreenEnabled
         ;
      },
  };
  
  /** @type {Object} */
  var specialHTMLTargets = [0, typeof document != 'undefined' ? document : 0, typeof window != 'undefined' ? window : 0];
  
  
  var maybeCStringToJsString = (cString) => {
      // "cString > 2" checks if the input is a number, and isn't of the special
      // values we accept here, EMSCRIPTEN_EVENT_TARGET_* (which map to 0, 1, 2).
      // In other words, if cString > 2 then it's a pointer to a valid place in
      // memory, and points to a C string.
      return cString > 2 ? UTF8ToString(cString) : cString;
    };
  
  /** @suppress {duplicate } */
  var findEventTarget = (target) => {
      target = maybeCStringToJsString(target);
      var domElement = specialHTMLTargets[target] || (typeof document != 'undefined' ? document.querySelector(target) : null);
      return domElement;
    };
  var findCanvasEventTarget = findEventTarget;
  var _emscripten_get_canvas_element_size = (target, width, height) => {
      var canvas = findCanvasEventTarget(target);
      if (!canvas) return -4;
      HEAP32[((width)>>2)] = canvas.width;
      HEAP32[((height)>>2)] = canvas.height;
    };
  
  
  
  
  
  var stackAlloc = (sz) => __emscripten_stack_alloc(sz);
  var stringToUTF8OnStack = (str) => {
      var size = lengthBytesUTF8(str) + 1;
      var ret = stackAlloc(size);
      stringToUTF8(str, ret, size);
      return ret;
    };
  var getCanvasElementSize = (target) => {
      var sp = stackSave();
      var w = stackAlloc(8);
      var h = w + 4;
  
      var targetInt = stringToUTF8OnStack(target.id);
      var ret = _emscripten_get_canvas_element_size(targetInt, w, h);
      var size = [HEAP32[((w)>>2)], HEAP32[((h)>>2)]];
      stackRestore(sp);
      return size;
    };
  
  var _emscripten_set_canvas_element_size = (target, width, height) => {
      var canvas = findCanvasEventTarget(target);
      if (!canvas) return -4;
      canvas.width = width;
      canvas.height = height;
      return 0;
    };
  
  
  
  var setCanvasElementSize = (target, width, height) => {
      if (!target.controlTransferredOffscreen) {
        target.width = width;
        target.height = height;
      } else {
        // This function is being called from high-level JavaScript code instead of asm.js/Wasm,
        // and it needs to synchronously proxy over to another thread, so marshal the string onto the heap to do the call.
        var sp = stackSave();
        var targetInt = stringToUTF8OnStack(target.id);
        _emscripten_set_canvas_element_size(targetInt, width, height);
        stackRestore(sp);
      }
    };
  
  var currentFullscreenStrategy = {
  };
  
  var wasmTableMirror = [];
  
  /** @type {WebAssembly.Table} */
  var wasmTable;
  var getWasmTableEntry = (funcPtr) => {
      var func = wasmTableMirror[funcPtr];
      if (!func) {
        /** @suppress {checkTypes} */
        wasmTableMirror[funcPtr] = func = wasmTable.get(funcPtr);
      }
      /** @suppress {checkTypes} */
      assert(wasmTable.get(funcPtr) == func, 'JavaScript-side Wasm function table mirror is out of date!');
      return func;
    };
  var registerRestoreOldStyle = (canvas) => {
      var canvasSize = getCanvasElementSize(canvas);
      var oldWidth = canvasSize[0];
      var oldHeight = canvasSize[1];
      var oldCssWidth = canvas.style.width;
      var oldCssHeight = canvas.style.height;
      var oldBackgroundColor = canvas.style.backgroundColor; // Chrome reads color from here.
      var oldDocumentBackgroundColor = document.body.style.backgroundColor; // IE11 reads color from here.
      // Firefox always has black background color.
      var oldPaddingLeft = canvas.style.paddingLeft; // Chrome, FF, Safari
      var oldPaddingRight = canvas.style.paddingRight;
      var oldPaddingTop = canvas.style.paddingTop;
      var oldPaddingBottom = canvas.style.paddingBottom;
      var oldMarginLeft = canvas.style.marginLeft; // IE11
      var oldMarginRight = canvas.style.marginRight;
      var oldMarginTop = canvas.style.marginTop;
      var oldMarginBottom = canvas.style.marginBottom;
      var oldDocumentBodyMargin = document.body.style.margin;
      var oldDocumentOverflow = document.documentElement.style.overflow; // Chrome, Firefox
      var oldDocumentScroll = document.body.scroll; // IE
      var oldImageRendering = canvas.style.imageRendering;
  
      function restoreOldStyle() {
        var fullscreenElement = document.fullscreenElement
          || document.webkitFullscreenElement
          ;
        if (!fullscreenElement) {
          document.removeEventListener('fullscreenchange', restoreOldStyle);
  
          // Unprefixed Fullscreen API shipped in Chromium 71 (https://bugs.chromium.org/p/chromium/issues/detail?id=383813)
          // As of Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitfullscreenchange. TODO: revisit this check once Safari ships unprefixed version.
          document.removeEventListener('webkitfullscreenchange', restoreOldStyle);
  
          setCanvasElementSize(canvas, oldWidth, oldHeight);
  
          canvas.style.width = oldCssWidth;
          canvas.style.height = oldCssHeight;
          canvas.style.backgroundColor = oldBackgroundColor; // Chrome
          // IE11 hack: assigning 'undefined' or an empty string to document.body.style.backgroundColor has no effect, so first assign back the default color
          // before setting the undefined value. Setting undefined value is also important, or otherwise we would later treat that as something that the user
          // had explicitly set so subsequent fullscreen transitions would not set background color properly.
          if (!oldDocumentBackgroundColor) document.body.style.backgroundColor = 'white';
          document.body.style.backgroundColor = oldDocumentBackgroundColor; // IE11
          canvas.style.paddingLeft = oldPaddingLeft; // Chrome, FF, Safari
          canvas.style.paddingRight = oldPaddingRight;
          canvas.style.paddingTop = oldPaddingTop;
          canvas.style.paddingBottom = oldPaddingBottom;
          canvas.style.marginLeft = oldMarginLeft; // IE11
          canvas.style.marginRight = oldMarginRight;
          canvas.style.marginTop = oldMarginTop;
          canvas.style.marginBottom = oldMarginBottom;
          document.body.style.margin = oldDocumentBodyMargin;
          document.documentElement.style.overflow = oldDocumentOverflow; // Chrome, Firefox
          document.body.scroll = oldDocumentScroll; // IE
          canvas.style.imageRendering = oldImageRendering;
          if (canvas.GLctxObject) canvas.GLctxObject.GLctx.viewport(0, 0, oldWidth, oldHeight);
  
          if (currentFullscreenStrategy.canvasResizedCallback) {
            getWasmTableEntry(currentFullscreenStrategy.canvasResizedCallback)(37, 0, currentFullscreenStrategy.canvasResizedCallbackUserData);
          }
        }
      }
      document.addEventListener('fullscreenchange', restoreOldStyle);
      // Unprefixed Fullscreen API shipped in Chromium 71 (https://bugs.chromium.org/p/chromium/issues/detail?id=383813)
      // As of Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitfullscreenchange. TODO: revisit this check once Safari ships unprefixed version.
      document.addEventListener('webkitfullscreenchange', restoreOldStyle);
      return restoreOldStyle;
    };
  
  
  var setLetterbox = (element, topBottom, leftRight) => {
      // Cannot use margin to specify letterboxes in FF or Chrome, since those ignore margins in fullscreen mode.
      element.style.paddingLeft = element.style.paddingRight = leftRight + 'px';
      element.style.paddingTop = element.style.paddingBottom = topBottom + 'px';
    };
  
  
  var getBoundingClientRect = (e) => specialHTMLTargets.indexOf(e) < 0 ? e.getBoundingClientRect() : {'left':0,'top':0};
  var JSEvents_resizeCanvasForFullscreen = (target, strategy) => {
      var restoreOldStyle = registerRestoreOldStyle(target);
      var cssWidth = strategy.softFullscreen ? innerWidth : screen.width;
      var cssHeight = strategy.softFullscreen ? innerHeight : screen.height;
      var rect = getBoundingClientRect(target);
      var windowedCssWidth = rect.width;
      var windowedCssHeight = rect.height;
      var canvasSize = getCanvasElementSize(target);
      var windowedRttWidth = canvasSize[0];
      var windowedRttHeight = canvasSize[1];
  
      if (strategy.scaleMode == 3) {
        setLetterbox(target, (cssHeight - windowedCssHeight) / 2, (cssWidth - windowedCssWidth) / 2);
        cssWidth = windowedCssWidth;
        cssHeight = windowedCssHeight;
      } else if (strategy.scaleMode == 2) {
        if (cssWidth*windowedRttHeight < windowedRttWidth*cssHeight) {
          var desiredCssHeight = windowedRttHeight * cssWidth / windowedRttWidth;
          setLetterbox(target, (cssHeight - desiredCssHeight) / 2, 0);
          cssHeight = desiredCssHeight;
        } else {
          var desiredCssWidth = windowedRttWidth * cssHeight / windowedRttHeight;
          setLetterbox(target, 0, (cssWidth - desiredCssWidth) / 2);
          cssWidth = desiredCssWidth;
        }
      }
  
      // If we are adding padding, must choose a background color or otherwise Chrome will give the
      // padding a default white color. Do it only if user has not customized their own background color.
      target.style.backgroundColor ||= 'black';
      // IE11 does the same, but requires the color to be set in the document body.
      document.body.style.backgroundColor ||= 'black'; // IE11
      // Firefox always shows black letterboxes independent of style color.
  
      target.style.width = cssWidth + 'px';
      target.style.height = cssHeight + 'px';
  
      if (strategy.filteringMode == 1) {
        target.style.imageRendering = 'optimizeSpeed';
        target.style.imageRendering = '-moz-crisp-edges';
        target.style.imageRendering = '-o-crisp-edges';
        target.style.imageRendering = '-webkit-optimize-contrast';
        target.style.imageRendering = 'optimize-contrast';
        target.style.imageRendering = 'crisp-edges';
        target.style.imageRendering = 'pixelated';
      }
  
      var dpiScale = (strategy.canvasResolutionScaleMode == 2) ? devicePixelRatio : 1;
      if (strategy.canvasResolutionScaleMode != 0) {
        var newWidth = (cssWidth * dpiScale)|0;
        var newHeight = (cssHeight * dpiScale)|0;
        setCanvasElementSize(target, newWidth, newHeight);
        if (target.GLctxObject) target.GLctxObject.GLctx.viewport(0, 0, newWidth, newHeight);
      }
      return restoreOldStyle;
    };
  
  var JSEvents_requestFullscreen = (target, strategy) => {
      // EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT + EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE is a mode where no extra logic is performed to the DOM elements.
      if (strategy.scaleMode != 0 || strategy.canvasResolutionScaleMode != 0) {
        JSEvents_resizeCanvasForFullscreen(target, strategy);
      }
  
      if (target.requestFullscreen) {
        target.requestFullscreen();
      } else if (target.webkitRequestFullscreen) {
        target.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT);
      } else {
        return JSEvents.fullscreenEnabled() ? -3 : -1;
      }
  
      currentFullscreenStrategy = strategy;
  
      if (strategy.canvasResizedCallback) {
        getWasmTableEntry(strategy.canvasResizedCallback)(37, 0, strategy.canvasResizedCallbackUserData);
      }
  
      return 0;
    };
  var _emscripten_exit_fullscreen = () => {
      if (!JSEvents.fullscreenEnabled()) return -1;
      // Make sure no queued up calls will fire after this.
      JSEvents.removeDeferredCalls(JSEvents_requestFullscreen);
  
      var d = specialHTMLTargets[1];
      if (d.exitFullscreen) {
        d.fullscreenElement && d.exitFullscreen();
      } else if (d.webkitExitFullscreen) {
        d.webkitFullscreenElement && d.webkitExitFullscreen();
      } else {
        return -1;
      }
  
      return 0;
    };

  
  var requestPointerLock = (target) => {
      if (target.requestPointerLock) {
        target.requestPointerLock();
      } else {
        // document.body is known to accept pointer lock, so use that to differentiate if the user passed a bad element,
        // or if the whole browser just doesn't support the feature.
        if (document.body.requestPointerLock
          ) {
          return -3;
        }
        return -1;
      }
      return 0;
    };
  var _emscripten_exit_pointerlock = () => {
      // Make sure no queued up calls will fire after this.
      JSEvents.removeDeferredCalls(requestPointerLock);
  
      if (document.exitPointerLock) {
        document.exitPointerLock();
      } else {
        return -1;
      }
      return 0;
    };

  function _emscripten_fetch_free(id) {
    if (Fetch.xhrs.has(id)) {
      var xhr = Fetch.xhrs.get(id);
      Fetch.xhrs.free(id);
      // check if fetch is still in progress and should be aborted
      if (xhr.readyState > 0 && xhr.readyState < 4) {
        xhr.abort();
      }
    }
  }


  var _emscripten_get_device_pixel_ratio = () => {
      return (typeof devicePixelRatio == 'number' && devicePixelRatio) || 1.0;
    };

  
  var _emscripten_get_element_css_size = (target, width, height) => {
      target = findEventTarget(target);
      if (!target) return -4;
  
      var rect = getBoundingClientRect(target);
      HEAPF64[((width)>>3)] = rect.width;
      HEAPF64[((height)>>3)] = rect.height;
  
      return 0;
    };

  
  var fillGamepadEventData = (eventStruct, e) => {
      HEAPF64[((eventStruct)>>3)] = e.timestamp;
      for (var i = 0; i < e.axes.length; ++i) {
        HEAPF64[(((eventStruct+i*8)+(16))>>3)] = e.axes[i];
      }
      for (var i = 0; i < e.buttons.length; ++i) {
        if (typeof e.buttons[i] == 'object') {
          HEAPF64[(((eventStruct+i*8)+(528))>>3)] = e.buttons[i].value;
        } else {
          HEAPF64[(((eventStruct+i*8)+(528))>>3)] = e.buttons[i];
        }
      }
      for (var i = 0; i < e.buttons.length; ++i) {
        if (typeof e.buttons[i] == 'object') {
          HEAP8[(eventStruct+i)+(1040)] = e.buttons[i].pressed;
        } else {
          // Assigning a boolean to HEAP32, that's ok, but Closure would like to warn about it:
          /** @suppress {checkTypes} */
          HEAP8[(eventStruct+i)+(1040)] = e.buttons[i] == 1;
        }
      }
      HEAP8[(eventStruct)+(1104)] = e.connected;
      HEAP32[(((eventStruct)+(1108))>>2)] = e.index;
      HEAP32[(((eventStruct)+(8))>>2)] = e.axes.length;
      HEAP32[(((eventStruct)+(12))>>2)] = e.buttons.length;
      stringToUTF8(e.id, eventStruct + 1112, 64);
      stringToUTF8(e.mapping, eventStruct + 1176, 64);
    };
  var _emscripten_get_gamepad_status = (index, gamepadState) => {
      if (!JSEvents.lastGamepadState) throw 'emscripten_get_gamepad_status() can only be called after having first called emscripten_sample_gamepad_data() and that function has returned EMSCRIPTEN_RESULT_SUCCESS!';
      // INVALID_PARAM is returned on a Gamepad index that never was there.
      if (index < 0 || index >= JSEvents.lastGamepadState.length) return -5;
  
      // NO_DATA is returned on a Gamepad index that was removed.
      // For previously disconnected gamepads there should be an empty slot (null/undefined/false) at the index.
      // This is because gamepads must keep their original position in the array.
      // For example, removing the first of two gamepads produces [null/undefined/false, gamepad].
      if (!JSEvents.lastGamepadState[index]) return -7;
  
      fillGamepadEventData(gamepadState, JSEvents.lastGamepadState[index]);
      return 0;
    };

  var getHeapMax = () =>
      // Stay one Wasm page short of 4GB: while e.g. Chrome is able to allocate
      // full 4GB Wasm memories, the size will wrap back to 0 bytes in Wasm side
      // for any code that deals with heap sizes, which would require special
      // casing all heap size related code to treat 0 specially.
      2147483648;
  var _emscripten_get_heap_max = () => getHeapMax();


  var _emscripten_get_num_gamepads = () => {
      if (!JSEvents.lastGamepadState) throw 'emscripten_get_num_gamepads() can only be called after having first called emscripten_sample_gamepad_data() and that function has returned EMSCRIPTEN_RESULT_SUCCESS!';
      // N.B. Do not call emscripten_get_num_gamepads() unless having first called emscripten_sample_gamepad_data(), and that has returned EMSCRIPTEN_RESULT_SUCCESS.
      // Otherwise the following line will throw an exception.
      return JSEvents.lastGamepadState.length;
    };

  var _emscripten_get_screen_size = (width, height) => {
      HEAP32[((width)>>2)] = screen.width;
      HEAP32[((height)>>2)] = screen.height;
    };

  /** @suppress {duplicate } */
  var _glActiveTexture = (x0) => GLctx.activeTexture(x0);
  var _emscripten_glActiveTexture = _glActiveTexture;

  /** @suppress {duplicate } */
  var _glAttachShader = (program, shader) => {
      GLctx.attachShader(GL.programs[program], GL.shaders[shader]);
    };
  var _emscripten_glAttachShader = _glAttachShader;

  /** @suppress {duplicate } */
  var _glBeginQuery = (target, id) => {
      GLctx.beginQuery(target, GL.queries[id]);
    };
  var _emscripten_glBeginQuery = _glBeginQuery;

  /** @suppress {duplicate } */
  var _glBeginQueryEXT = (target, id) => {
      GLctx.disjointTimerQueryExt['beginQueryEXT'](target, GL.queries[id]);
    };
  var _emscripten_glBeginQueryEXT = _glBeginQueryEXT;

  /** @suppress {duplicate } */
  var _glBeginTransformFeedback = (x0) => GLctx.beginTransformFeedback(x0);
  var _emscripten_glBeginTransformFeedback = _glBeginTransformFeedback;

  
  /** @suppress {duplicate } */
  var _glBindAttribLocation = (program, index, name) => {
      GLctx.bindAttribLocation(GL.programs[program], index, UTF8ToString(name));
    };
  var _emscripten_glBindAttribLocation = _glBindAttribLocation;

  /** @suppress {duplicate } */
  var _glBindBuffer = (target, buffer) => {
  
      if (target == 0x88EB /*GL_PIXEL_PACK_BUFFER*/) {
        // In WebGL 2 glReadPixels entry point, we need to use a different WebGL 2
        // API function call when a buffer is bound to
        // GL_PIXEL_PACK_BUFFER_BINDING point, so must keep track whether that
        // binding point is non-null to know what is the proper API function to
        // call.
        GLctx.currentPixelPackBufferBinding = buffer;
      } else if (target == 0x88EC /*GL_PIXEL_UNPACK_BUFFER*/) {
        // In WebGL 2 gl(Compressed)Tex(Sub)Image[23]D entry points, we need to
        // use a different WebGL 2 API function call when a buffer is bound to
        // GL_PIXEL_UNPACK_BUFFER_BINDING point, so must keep track whether that
        // binding point is non-null to know what is the proper API function to
        // call.
        GLctx.currentPixelUnpackBufferBinding = buffer;
      }
      GLctx.bindBuffer(target, GL.buffers[buffer]);
    };
  var _emscripten_glBindBuffer = _glBindBuffer;

  /** @suppress {duplicate } */
  var _glBindBufferBase = (target, index, buffer) => {
      GLctx.bindBufferBase(target, index, GL.buffers[buffer]);
    };
  var _emscripten_glBindBufferBase = _glBindBufferBase;

  /** @suppress {duplicate } */
  var _glBindBufferRange = (target, index, buffer, offset, ptrsize) => {
      GLctx.bindBufferRange(target, index, GL.buffers[buffer], offset, ptrsize);
    };
  var _emscripten_glBindBufferRange = _glBindBufferRange;

  /** @suppress {duplicate } */
  var _glBindFramebuffer = (target, framebuffer) => {
  
      GLctx.bindFramebuffer(target, GL.framebuffers[framebuffer]);
  
    };
  var _emscripten_glBindFramebuffer = _glBindFramebuffer;

  /** @suppress {duplicate } */
  var _glBindRenderbuffer = (target, renderbuffer) => {
      GLctx.bindRenderbuffer(target, GL.renderbuffers[renderbuffer]);
    };
  var _emscripten_glBindRenderbuffer = _glBindRenderbuffer;

  /** @suppress {duplicate } */
  var _glBindSampler = (unit, sampler) => {
      GLctx.bindSampler(unit, GL.samplers[sampler]);
    };
  var _emscripten_glBindSampler = _glBindSampler;

  /** @suppress {duplicate } */
  var _glBindTexture = (target, texture) => {
      GLctx.bindTexture(target, GL.textures[texture]);
    };
  var _emscripten_glBindTexture = _glBindTexture;

  /** @suppress {duplicate } */
  var _glBindTransformFeedback = (target, id) => {
      GLctx.bindTransformFeedback(target, GL.transformFeedbacks[id]);
    };
  var _emscripten_glBindTransformFeedback = _glBindTransformFeedback;

  /** @suppress {duplicate } */
  var _glBindVertexArray = (vao) => {
      GLctx.bindVertexArray(GL.vaos[vao]);
    };
  var _emscripten_glBindVertexArray = _glBindVertexArray;

  
  /** @suppress {duplicate } */
  var _glBindVertexArrayOES = _glBindVertexArray;
  var _emscripten_glBindVertexArrayOES = _glBindVertexArrayOES;

  /** @suppress {duplicate } */
  var _glBlendColor = (x0, x1, x2, x3) => GLctx.blendColor(x0, x1, x2, x3);
  var _emscripten_glBlendColor = _glBlendColor;

  /** @suppress {duplicate } */
  var _glBlendEquation = (x0) => GLctx.blendEquation(x0);
  var _emscripten_glBlendEquation = _glBlendEquation;

  /** @suppress {duplicate } */
  var _glBlendEquationSeparate = (x0, x1) => GLctx.blendEquationSeparate(x0, x1);
  var _emscripten_glBlendEquationSeparate = _glBlendEquationSeparate;

  /** @suppress {duplicate } */
  var _glBlendFunc = (x0, x1) => GLctx.blendFunc(x0, x1);
  var _emscripten_glBlendFunc = _glBlendFunc;

  /** @suppress {duplicate } */
  var _glBlendFuncSeparate = (x0, x1, x2, x3) => GLctx.blendFuncSeparate(x0, x1, x2, x3);
  var _emscripten_glBlendFuncSeparate = _glBlendFuncSeparate;

  /** @suppress {duplicate } */
  var _glBlitFramebuffer = (x0, x1, x2, x3, x4, x5, x6, x7, x8, x9) => GLctx.blitFramebuffer(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9);
  var _emscripten_glBlitFramebuffer = _glBlitFramebuffer;

  /** @suppress {duplicate } */
  var _glBufferData = (target, size, data, usage) => {
  
      if (true) {
        // If size is zero, WebGL would interpret uploading the whole input
        // arraybuffer (starting from given offset), which would not make sense in
        // WebAssembly, so avoid uploading if size is zero. However we must still
        // call bufferData to establish a backing storage of zero bytes.
        if (data && size) {
          GLctx.bufferData(target, HEAPU8, usage, data, size);
        } else {
          GLctx.bufferData(target, size, usage);
        }
        return;
      }
    };
  var _emscripten_glBufferData = _glBufferData;

  /** @suppress {duplicate } */
  var _glBufferSubData = (target, offset, size, data) => {
      if (true) {
        size && GLctx.bufferSubData(target, offset, HEAPU8, data, size);
        return;
      }
    };
  var _emscripten_glBufferSubData = _glBufferSubData;

  /** @suppress {duplicate } */
  var _glCheckFramebufferStatus = (x0) => GLctx.checkFramebufferStatus(x0);
  var _emscripten_glCheckFramebufferStatus = _glCheckFramebufferStatus;

  /** @suppress {duplicate } */
  var _glClear = (x0) => GLctx.clear(x0);
  var _emscripten_glClear = _glClear;

  /** @suppress {duplicate } */
  var _glClearBufferfi = (x0, x1, x2, x3) => GLctx.clearBufferfi(x0, x1, x2, x3);
  var _emscripten_glClearBufferfi = _glClearBufferfi;

  /** @suppress {duplicate } */
  var _glClearBufferfv = (buffer, drawbuffer, value) => {
  
      GLctx.clearBufferfv(buffer, drawbuffer, HEAPF32, ((value)>>2));
    };
  var _emscripten_glClearBufferfv = _glClearBufferfv;

  /** @suppress {duplicate } */
  var _glClearBufferiv = (buffer, drawbuffer, value) => {
  
      GLctx.clearBufferiv(buffer, drawbuffer, HEAP32, ((value)>>2));
    };
  var _emscripten_glClearBufferiv = _glClearBufferiv;

  /** @suppress {duplicate } */
  var _glClearBufferuiv = (buffer, drawbuffer, value) => {
  
      GLctx.clearBufferuiv(buffer, drawbuffer, HEAPU32, ((value)>>2));
    };
  var _emscripten_glClearBufferuiv = _glClearBufferuiv;

  /** @suppress {duplicate } */
  var _glClearColor = (x0, x1, x2, x3) => GLctx.clearColor(x0, x1, x2, x3);
  var _emscripten_glClearColor = _glClearColor;

  /** @suppress {duplicate } */
  var _glClearDepthf = (x0) => GLctx.clearDepth(x0);
  var _emscripten_glClearDepthf = _glClearDepthf;

  /** @suppress {duplicate } */
  var _glClearStencil = (x0) => GLctx.clearStencil(x0);
  var _emscripten_glClearStencil = _glClearStencil;

  /** @suppress {duplicate } */
  var _glClientWaitSync = (sync, flags, timeout) => {
      // WebGL2 vs GLES3 differences: in GLES3, the timeout parameter is a uint64, where 0xFFFFFFFFFFFFFFFFULL means GL_TIMEOUT_IGNORED.
      // In JS, there's no 64-bit value types, so instead timeout is taken to be signed, and GL_TIMEOUT_IGNORED is given value -1.
      // Inherently the value accepted in the timeout is lossy, and can't take in arbitrary u64 bit pattern (but most likely doesn't matter)
      // See https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.15
      timeout = Number(timeout);
      return GLctx.clientWaitSync(GL.syncs[sync], flags, timeout);
    };
  var _emscripten_glClientWaitSync = _glClientWaitSync;

  /** @suppress {duplicate } */
  var _glClipControlEXT = (origin, depth) => {
      GLctx.extClipControl['clipControlEXT'](origin, depth);
    };
  var _emscripten_glClipControlEXT = _glClipControlEXT;

  /** @suppress {duplicate } */
  var _glColorMask = (red, green, blue, alpha) => {
      GLctx.colorMask(!!red, !!green, !!blue, !!alpha);
    };
  var _emscripten_glColorMask = _glColorMask;

  /** @suppress {duplicate } */
  var _glCompileShader = (shader) => {
      GLctx.compileShader(GL.shaders[shader]);
    };
  var _emscripten_glCompileShader = _glCompileShader;

  /** @suppress {duplicate } */
  var _glCompressedTexImage2D = (target, level, internalFormat, width, height, border, imageSize, data) => {
      // `data` may be null here, which means "allocate uniniitalized space but
      // don't upload" in GLES parlance, but `compressedTexImage2D` requires the
      // final data parameter, so we simply pass a heap view starting at zero
      // effectively uploading whatever happens to be near address zero.  See
      // https://github.com/emscripten-core/emscripten/issues/19300.
      if (true) {
        if (GLctx.currentPixelUnpackBufferBinding || !imageSize) {
          GLctx.compressedTexImage2D(target, level, internalFormat, width, height, border, imageSize, data);
          return;
        }
        GLctx.compressedTexImage2D(target, level, internalFormat, width, height, border, HEAPU8, data, imageSize);
        return;
      }
    };
  var _emscripten_glCompressedTexImage2D = _glCompressedTexImage2D;

  /** @suppress {duplicate } */
  var _glCompressedTexImage3D = (target, level, internalFormat, width, height, depth, border, imageSize, data) => {
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx.compressedTexImage3D(target, level, internalFormat, width, height, depth, border, imageSize, data);
      } else {
        GLctx.compressedTexImage3D(target, level, internalFormat, width, height, depth, border, HEAPU8, data, imageSize);
      }
    };
  var _emscripten_glCompressedTexImage3D = _glCompressedTexImage3D;

  /** @suppress {duplicate } */
  var _glCompressedTexSubImage2D = (target, level, xoffset, yoffset, width, height, format, imageSize, data) => {
      if (true) {
        if (GLctx.currentPixelUnpackBufferBinding || !imageSize) {
          GLctx.compressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
          return;
        }
        GLctx.compressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, HEAPU8, data, imageSize);
        return;
      }
    };
  var _emscripten_glCompressedTexSubImage2D = _glCompressedTexSubImage2D;

  /** @suppress {duplicate } */
  var _glCompressedTexSubImage3D = (target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data) => {
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx.compressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
      } else {
        GLctx.compressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, HEAPU8, data, imageSize);
      }
    };
  var _emscripten_glCompressedTexSubImage3D = _glCompressedTexSubImage3D;

  /** @suppress {duplicate } */
  var _glCopyBufferSubData = (x0, x1, x2, x3, x4) => GLctx.copyBufferSubData(x0, x1, x2, x3, x4);
  var _emscripten_glCopyBufferSubData = _glCopyBufferSubData;

  /** @suppress {duplicate } */
  var _glCopyTexImage2D = (x0, x1, x2, x3, x4, x5, x6, x7) => GLctx.copyTexImage2D(x0, x1, x2, x3, x4, x5, x6, x7);
  var _emscripten_glCopyTexImage2D = _glCopyTexImage2D;

  /** @suppress {duplicate } */
  var _glCopyTexSubImage2D = (x0, x1, x2, x3, x4, x5, x6, x7) => GLctx.copyTexSubImage2D(x0, x1, x2, x3, x4, x5, x6, x7);
  var _emscripten_glCopyTexSubImage2D = _glCopyTexSubImage2D;

  /** @suppress {duplicate } */
  var _glCopyTexSubImage3D = (x0, x1, x2, x3, x4, x5, x6, x7, x8) => GLctx.copyTexSubImage3D(x0, x1, x2, x3, x4, x5, x6, x7, x8);
  var _emscripten_glCopyTexSubImage3D = _glCopyTexSubImage3D;

  /** @suppress {duplicate } */
  var _glCreateProgram = () => {
      var id = GL.getNewId(GL.programs);
      var program = GLctx.createProgram();
      // Store additional information needed for each shader program:
      program.name = id;
      // Lazy cache results of
      // glGetProgramiv(GL_ACTIVE_UNIFORM_MAX_LENGTH/GL_ACTIVE_ATTRIBUTE_MAX_LENGTH/GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH)
      program.maxUniformLength = program.maxAttributeLength = program.maxUniformBlockNameLength = 0;
      program.uniformIdCounter = 1;
      GL.programs[id] = program;
      return id;
    };
  var _emscripten_glCreateProgram = _glCreateProgram;

  /** @suppress {duplicate } */
  var _glCreateShader = (shaderType) => {
      var id = GL.getNewId(GL.shaders);
      GL.shaders[id] = GLctx.createShader(shaderType);
  
      return id;
    };
  var _emscripten_glCreateShader = _glCreateShader;

  /** @suppress {duplicate } */
  var _glCullFace = (x0) => GLctx.cullFace(x0);
  var _emscripten_glCullFace = _glCullFace;

  /** @suppress {duplicate } */
  var _glDeleteBuffers = (n, buffers) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((buffers)+(i*4))>>2)];
        var buffer = GL.buffers[id];
  
        // From spec: "glDeleteBuffers silently ignores 0's and names that do not
        // correspond to existing buffer objects."
        if (!buffer) continue;
  
        GLctx.deleteBuffer(buffer);
        buffer.name = 0;
        GL.buffers[id] = null;
  
        if (id == GLctx.currentPixelPackBufferBinding) GLctx.currentPixelPackBufferBinding = 0;
        if (id == GLctx.currentPixelUnpackBufferBinding) GLctx.currentPixelUnpackBufferBinding = 0;
      }
    };
  var _emscripten_glDeleteBuffers = _glDeleteBuffers;

  /** @suppress {duplicate } */
  var _glDeleteFramebuffers = (n, framebuffers) => {
      for (var i = 0; i < n; ++i) {
        var id = HEAP32[(((framebuffers)+(i*4))>>2)];
        var framebuffer = GL.framebuffers[id];
        if (!framebuffer) continue; // GL spec: "glDeleteFramebuffers silently ignores 0s and names that do not correspond to existing framebuffer objects".
        GLctx.deleteFramebuffer(framebuffer);
        framebuffer.name = 0;
        GL.framebuffers[id] = null;
      }
    };
  var _emscripten_glDeleteFramebuffers = _glDeleteFramebuffers;

  /** @suppress {duplicate } */
  var _glDeleteProgram = (id) => {
      if (!id) return;
      var program = GL.programs[id];
      if (!program) {
        // glDeleteProgram actually signals an error when deleting a nonexisting
        // object, unlike some other GL delete functions.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      GLctx.deleteProgram(program);
      program.name = 0;
      GL.programs[id] = null;
    };
  var _emscripten_glDeleteProgram = _glDeleteProgram;

  /** @suppress {duplicate } */
  var _glDeleteQueries = (n, ids) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((ids)+(i*4))>>2)];
        var query = GL.queries[id];
        if (!query) continue; // GL spec: "unused names in ids are ignored, as is the name zero."
        GLctx.deleteQuery(query);
        GL.queries[id] = null;
      }
    };
  var _emscripten_glDeleteQueries = _glDeleteQueries;

  /** @suppress {duplicate } */
  var _glDeleteQueriesEXT = (n, ids) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((ids)+(i*4))>>2)];
        var query = GL.queries[id];
        if (!query) continue; // GL spec: "unused names in ids are ignored, as is the name zero."
        GLctx.disjointTimerQueryExt['deleteQueryEXT'](query);
        GL.queries[id] = null;
      }
    };
  var _emscripten_glDeleteQueriesEXT = _glDeleteQueriesEXT;

  /** @suppress {duplicate } */
  var _glDeleteRenderbuffers = (n, renderbuffers) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((renderbuffers)+(i*4))>>2)];
        var renderbuffer = GL.renderbuffers[id];
        if (!renderbuffer) continue; // GL spec: "glDeleteRenderbuffers silently ignores 0s and names that do not correspond to existing renderbuffer objects".
        GLctx.deleteRenderbuffer(renderbuffer);
        renderbuffer.name = 0;
        GL.renderbuffers[id] = null;
      }
    };
  var _emscripten_glDeleteRenderbuffers = _glDeleteRenderbuffers;

  /** @suppress {duplicate } */
  var _glDeleteSamplers = (n, samplers) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((samplers)+(i*4))>>2)];
        var sampler = GL.samplers[id];
        if (!sampler) continue;
        GLctx.deleteSampler(sampler);
        sampler.name = 0;
        GL.samplers[id] = null;
      }
    };
  var _emscripten_glDeleteSamplers = _glDeleteSamplers;

  /** @suppress {duplicate } */
  var _glDeleteShader = (id) => {
      if (!id) return;
      var shader = GL.shaders[id];
      if (!shader) {
        // glDeleteShader actually signals an error when deleting a nonexisting
        // object, unlike some other GL delete functions.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      GLctx.deleteShader(shader);
      GL.shaders[id] = null;
    };
  var _emscripten_glDeleteShader = _glDeleteShader;

  /** @suppress {duplicate } */
  var _glDeleteSync = (id) => {
      if (!id) return;
      var sync = GL.syncs[id];
      if (!sync) { // glDeleteSync signals an error when deleting a nonexisting object, unlike some other GL delete functions.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      GLctx.deleteSync(sync);
      sync.name = 0;
      GL.syncs[id] = null;
    };
  var _emscripten_glDeleteSync = _glDeleteSync;

  /** @suppress {duplicate } */
  var _glDeleteTextures = (n, textures) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((textures)+(i*4))>>2)];
        var texture = GL.textures[id];
        // GL spec: "glDeleteTextures silently ignores 0s and names that do not
        // correspond to existing textures".
        if (!texture) continue;
        GLctx.deleteTexture(texture);
        texture.name = 0;
        GL.textures[id] = null;
      }
    };
  var _emscripten_glDeleteTextures = _glDeleteTextures;

  /** @suppress {duplicate } */
  var _glDeleteTransformFeedbacks = (n, ids) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((ids)+(i*4))>>2)];
        var transformFeedback = GL.transformFeedbacks[id];
        if (!transformFeedback) continue; // GL spec: "unused names in ids are ignored, as is the name zero."
        GLctx.deleteTransformFeedback(transformFeedback);
        transformFeedback.name = 0;
        GL.transformFeedbacks[id] = null;
      }
    };
  var _emscripten_glDeleteTransformFeedbacks = _glDeleteTransformFeedbacks;

  /** @suppress {duplicate } */
  var _glDeleteVertexArrays = (n, vaos) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((vaos)+(i*4))>>2)];
        GLctx.deleteVertexArray(GL.vaos[id]);
        GL.vaos[id] = null;
      }
    };
  var _emscripten_glDeleteVertexArrays = _glDeleteVertexArrays;

  
  /** @suppress {duplicate } */
  var _glDeleteVertexArraysOES = _glDeleteVertexArrays;
  var _emscripten_glDeleteVertexArraysOES = _glDeleteVertexArraysOES;

  /** @suppress {duplicate } */
  var _glDepthFunc = (x0) => GLctx.depthFunc(x0);
  var _emscripten_glDepthFunc = _glDepthFunc;

  /** @suppress {duplicate } */
  var _glDepthMask = (flag) => {
      GLctx.depthMask(!!flag);
    };
  var _emscripten_glDepthMask = _glDepthMask;

  /** @suppress {duplicate } */
  var _glDepthRangef = (x0, x1) => GLctx.depthRange(x0, x1);
  var _emscripten_glDepthRangef = _glDepthRangef;

  /** @suppress {duplicate } */
  var _glDetachShader = (program, shader) => {
      GLctx.detachShader(GL.programs[program], GL.shaders[shader]);
    };
  var _emscripten_glDetachShader = _glDetachShader;

  /** @suppress {duplicate } */
  var _glDisable = (x0) => GLctx.disable(x0);
  var _emscripten_glDisable = _glDisable;

  /** @suppress {duplicate } */
  var _glDisableVertexAttribArray = (index) => {
      GLctx.disableVertexAttribArray(index);
    };
  var _emscripten_glDisableVertexAttribArray = _glDisableVertexAttribArray;

  /** @suppress {duplicate } */
  var _glDrawArrays = (mode, first, count) => {
  
      GLctx.drawArrays(mode, first, count);
  
    };
  var _emscripten_glDrawArrays = _glDrawArrays;

  /** @suppress {duplicate } */
  var _glDrawArraysInstanced = (mode, first, count, primcount) => {
      GLctx.drawArraysInstanced(mode, first, count, primcount);
    };
  var _emscripten_glDrawArraysInstanced = _glDrawArraysInstanced;

  
  /** @suppress {duplicate } */
  var _glDrawArraysInstancedANGLE = _glDrawArraysInstanced;
  var _emscripten_glDrawArraysInstancedANGLE = _glDrawArraysInstancedANGLE;

  
  /** @suppress {duplicate } */
  var _glDrawArraysInstancedARB = _glDrawArraysInstanced;
  var _emscripten_glDrawArraysInstancedARB = _glDrawArraysInstancedARB;

  
  /** @suppress {duplicate } */
  var _glDrawArraysInstancedEXT = _glDrawArraysInstanced;
  var _emscripten_glDrawArraysInstancedEXT = _glDrawArraysInstancedEXT;

  
  /** @suppress {duplicate } */
  var _glDrawArraysInstancedNV = _glDrawArraysInstanced;
  var _emscripten_glDrawArraysInstancedNV = _glDrawArraysInstancedNV;

  var tempFixedLengthArray = [];
  
  /** @suppress {duplicate } */
  var _glDrawBuffers = (n, bufs) => {
  
      var bufArray = tempFixedLengthArray[n];
      for (var i = 0; i < n; i++) {
        bufArray[i] = HEAP32[(((bufs)+(i*4))>>2)];
      }
  
      GLctx.drawBuffers(bufArray);
    };
  var _emscripten_glDrawBuffers = _glDrawBuffers;

  
  /** @suppress {duplicate } */
  var _glDrawBuffersEXT = _glDrawBuffers;
  var _emscripten_glDrawBuffersEXT = _glDrawBuffersEXT;

  
  /** @suppress {duplicate } */
  var _glDrawBuffersWEBGL = _glDrawBuffers;
  var _emscripten_glDrawBuffersWEBGL = _glDrawBuffersWEBGL;

  /** @suppress {duplicate } */
  var _glDrawElements = (mode, count, type, indices) => {
  
      GLctx.drawElements(mode, count, type, indices);
  
    };
  var _emscripten_glDrawElements = _glDrawElements;

  /** @suppress {duplicate } */
  var _glDrawElementsInstanced = (mode, count, type, indices, primcount) => {
      GLctx.drawElementsInstanced(mode, count, type, indices, primcount);
    };
  var _emscripten_glDrawElementsInstanced = _glDrawElementsInstanced;

  
  /** @suppress {duplicate } */
  var _glDrawElementsInstancedANGLE = _glDrawElementsInstanced;
  var _emscripten_glDrawElementsInstancedANGLE = _glDrawElementsInstancedANGLE;

  
  /** @suppress {duplicate } */
  var _glDrawElementsInstancedARB = _glDrawElementsInstanced;
  var _emscripten_glDrawElementsInstancedARB = _glDrawElementsInstancedARB;

  
  /** @suppress {duplicate } */
  var _glDrawElementsInstancedEXT = _glDrawElementsInstanced;
  var _emscripten_glDrawElementsInstancedEXT = _glDrawElementsInstancedEXT;

  
  /** @suppress {duplicate } */
  var _glDrawElementsInstancedNV = _glDrawElementsInstanced;
  var _emscripten_glDrawElementsInstancedNV = _glDrawElementsInstancedNV;

  /** @suppress {duplicate } */
  var _glDrawRangeElements = (mode, start, end, count, type, indices) => {
      // TODO: This should be a trivial pass-though function registered at the bottom of this page as
      // glFuncs[6][1] += ' drawRangeElements';
      // but due to https://bugzilla.mozilla.org/show_bug.cgi?id=1202427,
      // we work around by ignoring the range.
      _glDrawElements(mode, count, type, indices);
    };
  var _emscripten_glDrawRangeElements = _glDrawRangeElements;

  /** @suppress {duplicate } */
  var _glEnable = (x0) => GLctx.enable(x0);
  var _emscripten_glEnable = _glEnable;

  /** @suppress {duplicate } */
  var _glEnableVertexAttribArray = (index) => {
      GLctx.enableVertexAttribArray(index);
    };
  var _emscripten_glEnableVertexAttribArray = _glEnableVertexAttribArray;

  /** @suppress {duplicate } */
  var _glEndQuery = (x0) => GLctx.endQuery(x0);
  var _emscripten_glEndQuery = _glEndQuery;

  /** @suppress {duplicate } */
  var _glEndQueryEXT = (target) => {
      GLctx.disjointTimerQueryExt['endQueryEXT'](target);
    };
  var _emscripten_glEndQueryEXT = _glEndQueryEXT;

  /** @suppress {duplicate } */
  var _glEndTransformFeedback = () => GLctx.endTransformFeedback();
  var _emscripten_glEndTransformFeedback = _glEndTransformFeedback;

  /** @suppress {duplicate } */
  var _glFenceSync = (condition, flags) => {
      var sync = GLctx.fenceSync(condition, flags);
      if (sync) {
        var id = GL.getNewId(GL.syncs);
        sync.name = id;
        GL.syncs[id] = sync;
        return id;
      }
      return 0; // Failed to create a sync object
    };
  var _emscripten_glFenceSync = _glFenceSync;

  /** @suppress {duplicate } */
  var _glFinish = () => GLctx.finish();
  var _emscripten_glFinish = _glFinish;

  /** @suppress {duplicate } */
  var _glFlush = () => GLctx.flush();
  var _emscripten_glFlush = _glFlush;

  /** @suppress {duplicate } */
  var _glFramebufferRenderbuffer = (target, attachment, renderbuffertarget, renderbuffer) => {
      GLctx.framebufferRenderbuffer(target, attachment, renderbuffertarget,
                                         GL.renderbuffers[renderbuffer]);
    };
  var _emscripten_glFramebufferRenderbuffer = _glFramebufferRenderbuffer;

  /** @suppress {duplicate } */
  var _glFramebufferTexture2D = (target, attachment, textarget, texture, level) => {
      GLctx.framebufferTexture2D(target, attachment, textarget,
                                      GL.textures[texture], level);
    };
  var _emscripten_glFramebufferTexture2D = _glFramebufferTexture2D;

  /** @suppress {duplicate } */
  var _glFramebufferTextureLayer = (target, attachment, texture, level, layer) => {
      GLctx.framebufferTextureLayer(target, attachment, GL.textures[texture], level, layer);
    };
  var _emscripten_glFramebufferTextureLayer = _glFramebufferTextureLayer;

  /** @suppress {duplicate } */
  var _glFrontFace = (x0) => GLctx.frontFace(x0);
  var _emscripten_glFrontFace = _glFrontFace;

  /** @suppress {duplicate } */
  var _glGenBuffers = (n, buffers) => {
      GL.genObject(n, buffers, 'createBuffer', GL.buffers
        );
    };
  var _emscripten_glGenBuffers = _glGenBuffers;

  /** @suppress {duplicate } */
  var _glGenFramebuffers = (n, ids) => {
      GL.genObject(n, ids, 'createFramebuffer', GL.framebuffers
        );
    };
  var _emscripten_glGenFramebuffers = _glGenFramebuffers;

  /** @suppress {duplicate } */
  var _glGenQueries = (n, ids) => {
      GL.genObject(n, ids, 'createQuery', GL.queries
        );
    };
  var _emscripten_glGenQueries = _glGenQueries;

  /** @suppress {duplicate } */
  var _glGenQueriesEXT = (n, ids) => {
      for (var i = 0; i < n; i++) {
        var query = GLctx.disjointTimerQueryExt['createQueryEXT']();
        if (!query) {
          GL.recordError(0x502 /* GL_INVALID_OPERATION */);
          while (i < n) HEAP32[(((ids)+(i++*4))>>2)] = 0;
          return;
        }
        var id = GL.getNewId(GL.queries);
        query.name = id;
        GL.queries[id] = query;
        HEAP32[(((ids)+(i*4))>>2)] = id;
      }
    };
  var _emscripten_glGenQueriesEXT = _glGenQueriesEXT;

  /** @suppress {duplicate } */
  var _glGenRenderbuffers = (n, renderbuffers) => {
      GL.genObject(n, renderbuffers, 'createRenderbuffer', GL.renderbuffers
        );
    };
  var _emscripten_glGenRenderbuffers = _glGenRenderbuffers;

  /** @suppress {duplicate } */
  var _glGenSamplers = (n, samplers) => {
      GL.genObject(n, samplers, 'createSampler', GL.samplers
        );
    };
  var _emscripten_glGenSamplers = _glGenSamplers;

  /** @suppress {duplicate } */
  var _glGenTextures = (n, textures) => {
      GL.genObject(n, textures, 'createTexture', GL.textures
        );
    };
  var _emscripten_glGenTextures = _glGenTextures;

  /** @suppress {duplicate } */
  var _glGenTransformFeedbacks = (n, ids) => {
      GL.genObject(n, ids, 'createTransformFeedback', GL.transformFeedbacks
        );
    };
  var _emscripten_glGenTransformFeedbacks = _glGenTransformFeedbacks;

  /** @suppress {duplicate } */
  var _glGenVertexArrays = (n, arrays) => {
      GL.genObject(n, arrays, 'createVertexArray', GL.vaos
        );
    };
  var _emscripten_glGenVertexArrays = _glGenVertexArrays;

  
  /** @suppress {duplicate } */
  var _glGenVertexArraysOES = _glGenVertexArrays;
  var _emscripten_glGenVertexArraysOES = _glGenVertexArraysOES;

  /** @suppress {duplicate } */
  var _glGenerateMipmap = (x0) => GLctx.generateMipmap(x0);
  var _emscripten_glGenerateMipmap = _glGenerateMipmap;

  
  var __glGetActiveAttribOrUniform = (funcName, program, index, bufSize, length, size, type, name) => {
      program = GL.programs[program];
      var info = GLctx[funcName](program, index);
      if (info) {
        // If an error occurs, nothing will be written to length, size and type and name.
        var numBytesWrittenExclNull = name && stringToUTF8(info.name, name, bufSize);
        if (length) HEAP32[((length)>>2)] = numBytesWrittenExclNull;
        if (size) HEAP32[((size)>>2)] = info.size;
        if (type) HEAP32[((type)>>2)] = info.type;
      }
    };
  
  /** @suppress {duplicate } */
  var _glGetActiveAttrib = (program, index, bufSize, length, size, type, name) =>
      __glGetActiveAttribOrUniform('getActiveAttrib', program, index, bufSize, length, size, type, name);
  var _emscripten_glGetActiveAttrib = _glGetActiveAttrib;

  
  /** @suppress {duplicate } */
  var _glGetActiveUniform = (program, index, bufSize, length, size, type, name) =>
      __glGetActiveAttribOrUniform('getActiveUniform', program, index, bufSize, length, size, type, name);
  var _emscripten_glGetActiveUniform = _glGetActiveUniform;

  /** @suppress {duplicate } */
  var _glGetActiveUniformBlockName = (program, uniformBlockIndex, bufSize, length, uniformBlockName) => {
      program = GL.programs[program];
  
      var result = GLctx.getActiveUniformBlockName(program, uniformBlockIndex);
      if (!result) return; // If an error occurs, nothing will be written to uniformBlockName or length.
      if (uniformBlockName && bufSize > 0) {
        var numBytesWrittenExclNull = stringToUTF8(result, uniformBlockName, bufSize);
        if (length) HEAP32[((length)>>2)] = numBytesWrittenExclNull;
      } else {
        if (length) HEAP32[((length)>>2)] = 0;
      }
    };
  var _emscripten_glGetActiveUniformBlockName = _glGetActiveUniformBlockName;

  /** @suppress {duplicate } */
  var _glGetActiveUniformBlockiv = (program, uniformBlockIndex, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if params == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      program = GL.programs[program];
  
      if (pname == 0x8A41 /* GL_UNIFORM_BLOCK_NAME_LENGTH */) {
        var name = GLctx.getActiveUniformBlockName(program, uniformBlockIndex);
        HEAP32[((params)>>2)] = name.length+1;
        return;
      }
  
      var result = GLctx.getActiveUniformBlockParameter(program, uniformBlockIndex, pname);
      if (result === null) return; // If an error occurs, nothing should be written to params.
      if (pname == 0x8A43 /*GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES*/) {
        for (var i = 0; i < result.length; i++) {
          HEAP32[(((params)+(i*4))>>2)] = result[i];
        }
      } else {
        HEAP32[((params)>>2)] = result;
      }
    };
  var _emscripten_glGetActiveUniformBlockiv = _glGetActiveUniformBlockiv;

  /** @suppress {duplicate } */
  var _glGetActiveUniformsiv = (program, uniformCount, uniformIndices, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if params == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      if (uniformCount > 0 && uniformIndices == 0) {
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      program = GL.programs[program];
      var ids = [];
      for (var i = 0; i < uniformCount; i++) {
        ids.push(HEAP32[(((uniformIndices)+(i*4))>>2)]);
      }
  
      var result = GLctx.getActiveUniforms(program, ids, pname);
      if (!result) return; // GL spec: If an error is generated, nothing is written out to params.
  
      var len = result.length;
      for (var i = 0; i < len; i++) {
        HEAP32[(((params)+(i*4))>>2)] = result[i];
      }
    };
  var _emscripten_glGetActiveUniformsiv = _glGetActiveUniformsiv;

  /** @suppress {duplicate } */
  var _glGetAttachedShaders = (program, maxCount, count, shaders) => {
      var result = GLctx.getAttachedShaders(GL.programs[program]);
      var len = result.length;
      if (len > maxCount) {
        len = maxCount;
      }
      HEAP32[((count)>>2)] = len;
      for (var i = 0; i < len; ++i) {
        var id = GL.shaders.indexOf(result[i]);
        HEAP32[(((shaders)+(i*4))>>2)] = id;
      }
    };
  var _emscripten_glGetAttachedShaders = _glGetAttachedShaders;

  
  /** @suppress {duplicate } */
  var _glGetAttribLocation = (program, name) =>
      GLctx.getAttribLocation(GL.programs[program], UTF8ToString(name));
  var _emscripten_glGetAttribLocation = _glGetAttribLocation;

  var readI53FromI64 = (ptr) => {
      return HEAPU32[((ptr)>>2)] + HEAP32[(((ptr)+(4))>>2)] * 4294967296;
    };
  
  var readI53FromU64 = (ptr) => {
      return HEAPU32[((ptr)>>2)] + HEAPU32[(((ptr)+(4))>>2)] * 4294967296;
    };
  var writeI53ToI64 = (ptr, num) => {
      HEAPU32[((ptr)>>2)] = num;
      var lower = HEAPU32[((ptr)>>2)];
      HEAPU32[(((ptr)+(4))>>2)] = (num - lower)/4294967296;
      var deserialized = (num >= 0) ? readI53FromU64(ptr) : readI53FromI64(ptr);
      var offset = ((ptr)>>2);
      if (deserialized != num) warnOnce(`writeI53ToI64() out of range: serialized JS Number ${num} to Wasm heap as bytes lo=${ptrToString(HEAPU32[offset])}, hi=${ptrToString(HEAPU32[offset+1])}, which deserializes back to ${deserialized} instead!`);
    };
  
  
  var webglGetExtensions = () => {
      var exts = getEmscriptenSupportedExtensions(GLctx);
      exts = exts.concat(exts.map((e) => "GL_" + e));
      return exts;
    };
  
  var emscriptenWebGLGet = (name_, p, type) => {
      // Guard against user passing a null pointer.
      // Note that GLES2 spec does not say anything about how passing a null
      // pointer should be treated.  Testing on desktop core GL 3, the application
      // crashes on glGetIntegerv to a null pointer, but better to report an error
      // instead of doing anything random.
      if (!p) {
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var ret = undefined;
      switch (name_) { // Handle a few trivial GLES values
        case 0x8DFA: // GL_SHADER_COMPILER
          ret = 1;
          break;
        case 0x8DF8: // GL_SHADER_BINARY_FORMATS
          if (type != 0 && type != 1) {
            GL.recordError(0x500); // GL_INVALID_ENUM
          }
          // Do not write anything to the out pointer, since no binary formats are
          // supported.
          return;
        case 0x87FE: // GL_NUM_PROGRAM_BINARY_FORMATS
        case 0x8DF9: // GL_NUM_SHADER_BINARY_FORMATS
          ret = 0;
          break;
        case 0x86A2: // GL_NUM_COMPRESSED_TEXTURE_FORMATS
          // WebGL doesn't have GL_NUM_COMPRESSED_TEXTURE_FORMATS (it's obsolete
          // since GL_COMPRESSED_TEXTURE_FORMATS returns a JS array that can be
          // queried for length), so implement it ourselves to allow C++ GLES2
          // code get the length.
          var formats = GLctx.getParameter(0x86A3 /*GL_COMPRESSED_TEXTURE_FORMATS*/);
          ret = formats ? formats.length : 0;
          break;
  
        case 0x821D: // GL_NUM_EXTENSIONS
          if (GL.currentContext.version < 2) {
            // Calling GLES3/WebGL2 function with a GLES2/WebGL1 context
            GL.recordError(0x502 /* GL_INVALID_OPERATION */);
            return;
          }
          ret = webglGetExtensions().length;
          break;
        case 0x821B: // GL_MAJOR_VERSION
        case 0x821C: // GL_MINOR_VERSION
          if (GL.currentContext.version < 2) {
            GL.recordError(0x500); // GL_INVALID_ENUM
            return;
          }
          ret = name_ == 0x821B ? 3 : 0; // return version 3.0
          break;
      }
  
      if (ret === undefined) {
        var result = GLctx.getParameter(name_);
        switch (typeof result) {
          case "number":
            ret = result;
            break;
          case "boolean":
            ret = result ? 1 : 0;
            break;
          case "string":
            GL.recordError(0x500); // GL_INVALID_ENUM
            return;
          case "object":
            if (result === null) {
              // null is a valid result for some (e.g., which buffer is bound -
              // perhaps nothing is bound), but otherwise can mean an invalid
              // name_, which we need to report as an error
              switch (name_) {
                case 0x8894: // ARRAY_BUFFER_BINDING
                case 0x8B8D: // CURRENT_PROGRAM
                case 0x8895: // ELEMENT_ARRAY_BUFFER_BINDING
                case 0x8CA6: // FRAMEBUFFER_BINDING or DRAW_FRAMEBUFFER_BINDING
                case 0x8CA7: // RENDERBUFFER_BINDING
                case 0x8069: // TEXTURE_BINDING_2D
                case 0x85B5: // WebGL 2 GL_VERTEX_ARRAY_BINDING, or WebGL 1 extension OES_vertex_array_object GL_VERTEX_ARRAY_BINDING_OES
                case 0x8F36: // COPY_READ_BUFFER_BINDING or COPY_READ_BUFFER
                case 0x8F37: // COPY_WRITE_BUFFER_BINDING or COPY_WRITE_BUFFER
                case 0x88ED: // PIXEL_PACK_BUFFER_BINDING
                case 0x88EF: // PIXEL_UNPACK_BUFFER_BINDING
                case 0x8CAA: // READ_FRAMEBUFFER_BINDING
                case 0x8919: // SAMPLER_BINDING
                case 0x8C1D: // TEXTURE_BINDING_2D_ARRAY
                case 0x806A: // TEXTURE_BINDING_3D
                case 0x8E25: // TRANSFORM_FEEDBACK_BINDING
                case 0x8C8F: // TRANSFORM_FEEDBACK_BUFFER_BINDING
                case 0x8A28: // UNIFORM_BUFFER_BINDING
                case 0x8514: { // TEXTURE_BINDING_CUBE_MAP
                  ret = 0;
                  break;
                }
                default: {
                  GL.recordError(0x500); // GL_INVALID_ENUM
                  return;
                }
              }
            } else if (result instanceof Float32Array ||
                       result instanceof Uint32Array ||
                       result instanceof Int32Array ||
                       result instanceof Array) {
              for (var i = 0; i < result.length; ++i) {
                switch (type) {
                  case 0: HEAP32[(((p)+(i*4))>>2)] = result[i]; break;
                  case 2: HEAPF32[(((p)+(i*4))>>2)] = result[i]; break;
                  case 4: HEAP8[(p)+(i)] = result[i] ? 1 : 0; break;
                }
              }
              return;
            } else {
              try {
                ret = result.name | 0;
              } catch(e) {
                GL.recordError(0x500); // GL_INVALID_ENUM
                err(`GL_INVALID_ENUM in glGet${type}v: Unknown object returned from WebGL getParameter(${name_})! (error: ${e})`);
                return;
              }
            }
            break;
          default:
            GL.recordError(0x500); // GL_INVALID_ENUM
            err(`GL_INVALID_ENUM in glGet${type}v: Native code calling glGet${type}v(${name_}) and it returns ${result} of type ${typeof(result)}!`);
            return;
        }
      }
  
      switch (type) {
        case 1: writeI53ToI64(p, ret); break;
        case 0: HEAP32[((p)>>2)] = ret; break;
        case 2:   HEAPF32[((p)>>2)] = ret; break;
        case 4: HEAP8[p] = ret ? 1 : 0; break;
      }
    };
  
  /** @suppress {duplicate } */
  var _glGetBooleanv = (name_, p) => emscriptenWebGLGet(name_, p, 4);
  var _emscripten_glGetBooleanv = _glGetBooleanv;

  /** @suppress {duplicate } */
  var _glGetBufferParameteri64v = (target, value, data) => {
      if (!data) {
        // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
        // if data == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      writeI53ToI64(data, GLctx.getBufferParameter(target, value));
    };
  var _emscripten_glGetBufferParameteri64v = _glGetBufferParameteri64v;

  /** @suppress {duplicate } */
  var _glGetBufferParameteriv = (target, value, data) => {
      if (!data) {
        // GLES2 specification does not specify how to behave if data is a null
        // pointer. Since calling this function does not make sense if data ==
        // null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((data)>>2)] = GLctx.getBufferParameter(target, value);
    };
  var _emscripten_glGetBufferParameteriv = _glGetBufferParameteriv;

  /** @suppress {duplicate } */
  var _glGetError = () => {
      var error = GLctx.getError() || GL.lastError;
      GL.lastError = 0/*GL_NO_ERROR*/;
      return error;
    };
  var _emscripten_glGetError = _glGetError;

  
  /** @suppress {duplicate } */
  var _glGetFloatv = (name_, p) => emscriptenWebGLGet(name_, p, 2);
  var _emscripten_glGetFloatv = _glGetFloatv;

  /** @suppress {duplicate } */
  var _glGetFragDataLocation = (program, name) => {
      return GLctx.getFragDataLocation(GL.programs[program], UTF8ToString(name));
    };
  var _emscripten_glGetFragDataLocation = _glGetFragDataLocation;

  /** @suppress {duplicate } */
  var _glGetFramebufferAttachmentParameteriv = (target, attachment, pname, params) => {
      var result = GLctx.getFramebufferAttachmentParameter(target, attachment, pname);
      if (result instanceof WebGLRenderbuffer ||
          result instanceof WebGLTexture) {
        result = result.name | 0;
      }
      HEAP32[((params)>>2)] = result;
    };
  var _emscripten_glGetFramebufferAttachmentParameteriv = _glGetFramebufferAttachmentParameteriv;

  var emscriptenWebGLGetIndexed = (target, index, data, type) => {
      if (!data) {
        // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
        // if data == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var result = GLctx.getIndexedParameter(target, index);
      var ret;
      switch (typeof result) {
        case 'boolean':
          ret = result ? 1 : 0;
          break;
        case 'number':
          ret = result;
          break;
        case 'object':
          if (result === null) {
            switch (target) {
              case 0x8C8F: // TRANSFORM_FEEDBACK_BUFFER_BINDING
              case 0x8A28: // UNIFORM_BUFFER_BINDING
                ret = 0;
                break;
              default: {
                GL.recordError(0x500); // GL_INVALID_ENUM
                return;
              }
            }
          } else if (result instanceof WebGLBuffer) {
            ret = result.name | 0;
          } else {
            GL.recordError(0x500); // GL_INVALID_ENUM
            return;
          }
          break;
        default:
          GL.recordError(0x500); // GL_INVALID_ENUM
          return;
      }
  
      switch (type) {
        case 1: writeI53ToI64(data, ret); break;
        case 0: HEAP32[((data)>>2)] = ret; break;
        case 2: HEAPF32[((data)>>2)] = ret; break;
        case 4: HEAP8[data] = ret ? 1 : 0; break;
        default: throw 'internal emscriptenWebGLGetIndexed() error, bad type: ' + type;
      }
    };
  /** @suppress {duplicate } */
  var _glGetInteger64i_v = (target, index, data) =>
      emscriptenWebGLGetIndexed(target, index, data, 1);
  var _emscripten_glGetInteger64i_v = _glGetInteger64i_v;

  /** @suppress {duplicate } */
  var _glGetInteger64v = (name_, p) => {
      emscriptenWebGLGet(name_, p, 1);
    };
  var _emscripten_glGetInteger64v = _glGetInteger64v;

  /** @suppress {duplicate } */
  var _glGetIntegeri_v = (target, index, data) =>
      emscriptenWebGLGetIndexed(target, index, data, 0);
  var _emscripten_glGetIntegeri_v = _glGetIntegeri_v;

  
  /** @suppress {duplicate } */
  var _glGetIntegerv = (name_, p) => emscriptenWebGLGet(name_, p, 0);
  var _emscripten_glGetIntegerv = _glGetIntegerv;

  /** @suppress {duplicate } */
  var _glGetInternalformativ = (target, internalformat, pname, bufSize, params) => {
      if (bufSize < 0) {
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      if (!params) {
        // GLES3 specification does not specify how to behave if values is a null pointer. Since calling this function does not make sense
        // if values == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var ret = GLctx.getInternalformatParameter(target, internalformat, pname);
      if (ret === null) return;
      for (var i = 0; i < ret.length && i < bufSize; ++i) {
        HEAP32[(((params)+(i*4))>>2)] = ret[i];
      }
    };
  var _emscripten_glGetInternalformativ = _glGetInternalformativ;

  /** @suppress {duplicate } */
  var _glGetProgramBinary = (program, bufSize, length, binaryFormat, binary) => {
      GL.recordError(0x502/*GL_INVALID_OPERATION*/);
    };
  var _emscripten_glGetProgramBinary = _glGetProgramBinary;

  /** @suppress {duplicate } */
  var _glGetProgramInfoLog = (program, maxLength, length, infoLog) => {
      var log = GLctx.getProgramInfoLog(GL.programs[program]);
      if (log === null) log = '(unknown error)';
      var numBytesWrittenExclNull = (maxLength > 0 && infoLog) ? stringToUTF8(log, infoLog, maxLength) : 0;
      if (length) HEAP32[((length)>>2)] = numBytesWrittenExclNull;
    };
  var _emscripten_glGetProgramInfoLog = _glGetProgramInfoLog;

  /** @suppress {duplicate } */
  var _glGetProgramiv = (program, pname, p) => {
      if (!p) {
        // GLES2 specification does not specify how to behave if p is a null
        // pointer. Since calling this function does not make sense if p == null,
        // issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
  
      if (program >= GL.counter) {
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
  
      program = GL.programs[program];
  
      if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
        var log = GLctx.getProgramInfoLog(program);
        if (log === null) log = '(unknown error)';
        HEAP32[((p)>>2)] = log.length + 1;
      } else if (pname == 0x8B87 /* GL_ACTIVE_UNIFORM_MAX_LENGTH */) {
        if (!program.maxUniformLength) {
          var numActiveUniforms = GLctx.getProgramParameter(program, 0x8B86/*GL_ACTIVE_UNIFORMS*/);
          for (var i = 0; i < numActiveUniforms; ++i) {
            program.maxUniformLength = Math.max(program.maxUniformLength, GLctx.getActiveUniform(program, i).name.length+1);
          }
        }
        HEAP32[((p)>>2)] = program.maxUniformLength;
      } else if (pname == 0x8B8A /* GL_ACTIVE_ATTRIBUTE_MAX_LENGTH */) {
        if (!program.maxAttributeLength) {
          var numActiveAttributes = GLctx.getProgramParameter(program, 0x8B89/*GL_ACTIVE_ATTRIBUTES*/);
          for (var i = 0; i < numActiveAttributes; ++i) {
            program.maxAttributeLength = Math.max(program.maxAttributeLength, GLctx.getActiveAttrib(program, i).name.length+1);
          }
        }
        HEAP32[((p)>>2)] = program.maxAttributeLength;
      } else if (pname == 0x8A35 /* GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH */) {
        if (!program.maxUniformBlockNameLength) {
          var numActiveUniformBlocks = GLctx.getProgramParameter(program, 0x8A36/*GL_ACTIVE_UNIFORM_BLOCKS*/);
          for (var i = 0; i < numActiveUniformBlocks; ++i) {
            program.maxUniformBlockNameLength = Math.max(program.maxUniformBlockNameLength, GLctx.getActiveUniformBlockName(program, i).length+1);
          }
        }
        HEAP32[((p)>>2)] = program.maxUniformBlockNameLength;
      } else {
        HEAP32[((p)>>2)] = GLctx.getProgramParameter(program, pname);
      }
    };
  var _emscripten_glGetProgramiv = _glGetProgramiv;

  
  /** @suppress {duplicate } */
  var _glGetQueryObjecti64vEXT = (id, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var query = GL.queries[id];
      var param;
      if (GL.currentContext.version < 2)
      {
        param = GLctx.disjointTimerQueryExt['getQueryObjectEXT'](query, pname);
      }
      else {
        param = GLctx.getQueryParameter(query, pname);
      }
      var ret;
      if (typeof param == 'boolean') {
        ret = param ? 1 : 0;
      } else {
        ret = param;
      }
      writeI53ToI64(params, ret);
    };
  var _emscripten_glGetQueryObjecti64vEXT = _glGetQueryObjecti64vEXT;

  /** @suppress {duplicate } */
  var _glGetQueryObjectivEXT = (id, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var query = GL.queries[id];
      var param = GLctx.disjointTimerQueryExt['getQueryObjectEXT'](query, pname);
      var ret;
      if (typeof param == 'boolean') {
        ret = param ? 1 : 0;
      } else {
        ret = param;
      }
      HEAP32[((params)>>2)] = ret;
    };
  var _emscripten_glGetQueryObjectivEXT = _glGetQueryObjectivEXT;

  
  /** @suppress {duplicate } */
  var _glGetQueryObjectui64vEXT = _glGetQueryObjecti64vEXT;
  var _emscripten_glGetQueryObjectui64vEXT = _glGetQueryObjectui64vEXT;

  /** @suppress {duplicate } */
  var _glGetQueryObjectuiv = (id, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var query = GL.queries[id];
      var param = GLctx.getQueryParameter(query, pname);
      var ret;
      if (typeof param == 'boolean') {
        ret = param ? 1 : 0;
      } else {
        ret = param;
      }
      HEAP32[((params)>>2)] = ret;
    };
  var _emscripten_glGetQueryObjectuiv = _glGetQueryObjectuiv;

  
  /** @suppress {duplicate } */
  var _glGetQueryObjectuivEXT = _glGetQueryObjectivEXT;
  var _emscripten_glGetQueryObjectuivEXT = _glGetQueryObjectuivEXT;

  /** @suppress {duplicate } */
  var _glGetQueryiv = (target, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((params)>>2)] = GLctx.getQuery(target, pname);
    };
  var _emscripten_glGetQueryiv = _glGetQueryiv;

  /** @suppress {duplicate } */
  var _glGetQueryivEXT = (target, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((params)>>2)] = GLctx.disjointTimerQueryExt['getQueryEXT'](target, pname);
    };
  var _emscripten_glGetQueryivEXT = _glGetQueryivEXT;

  /** @suppress {duplicate } */
  var _glGetRenderbufferParameteriv = (target, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if params == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((params)>>2)] = GLctx.getRenderbufferParameter(target, pname);
    };
  var _emscripten_glGetRenderbufferParameteriv = _glGetRenderbufferParameteriv;

  /** @suppress {duplicate } */
  var _glGetSamplerParameterfv = (sampler, pname, params) => {
      if (!params) {
        // GLES3 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAPF32[((params)>>2)] = GLctx.getSamplerParameter(GL.samplers[sampler], pname);
    };
  var _emscripten_glGetSamplerParameterfv = _glGetSamplerParameterfv;

  /** @suppress {duplicate } */
  var _glGetSamplerParameteriv = (sampler, pname, params) => {
      if (!params) {
        // GLES3 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((params)>>2)] = GLctx.getSamplerParameter(GL.samplers[sampler], pname);
    };
  var _emscripten_glGetSamplerParameteriv = _glGetSamplerParameteriv;

  
  /** @suppress {duplicate } */
  var _glGetShaderInfoLog = (shader, maxLength, length, infoLog) => {
      var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
      if (log === null) log = '(unknown error)';
      var numBytesWrittenExclNull = (maxLength > 0 && infoLog) ? stringToUTF8(log, infoLog, maxLength) : 0;
      if (length) HEAP32[((length)>>2)] = numBytesWrittenExclNull;
    };
  var _emscripten_glGetShaderInfoLog = _glGetShaderInfoLog;

  /** @suppress {duplicate } */
  var _glGetShaderPrecisionFormat = (shaderType, precisionType, range, precision) => {
      var result = GLctx.getShaderPrecisionFormat(shaderType, precisionType);
      HEAP32[((range)>>2)] = result.rangeMin;
      HEAP32[(((range)+(4))>>2)] = result.rangeMax;
      HEAP32[((precision)>>2)] = result.precision;
    };
  var _emscripten_glGetShaderPrecisionFormat = _glGetShaderPrecisionFormat;

  /** @suppress {duplicate } */
  var _glGetShaderSource = (shader, bufSize, length, source) => {
      var result = GLctx.getShaderSource(GL.shaders[shader]);
      if (!result) return; // If an error occurs, nothing will be written to length or source.
      var numBytesWrittenExclNull = (bufSize > 0 && source) ? stringToUTF8(result, source, bufSize) : 0;
      if (length) HEAP32[((length)>>2)] = numBytesWrittenExclNull;
    };
  var _emscripten_glGetShaderSource = _glGetShaderSource;

  /** @suppress {duplicate } */
  var _glGetShaderiv = (shader, pname, p) => {
      if (!p) {
        // GLES2 specification does not specify how to behave if p is a null
        // pointer. Since calling this function does not make sense if p == null,
        // issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
        var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
        if (log === null) log = '(unknown error)';
        // The GLES2 specification says that if the shader has an empty info log,
        // a value of 0 is returned. Otherwise the log has a null char appended.
        // (An empty string is falsey, so we can just check that instead of
        // looking at log.length.)
        var logLength = log ? log.length + 1 : 0;
        HEAP32[((p)>>2)] = logLength;
      } else if (pname == 0x8B88) { // GL_SHADER_SOURCE_LENGTH
        var source = GLctx.getShaderSource(GL.shaders[shader]);
        // source may be a null, or the empty string, both of which are falsey
        // values that we report a 0 length for.
        var sourceLength = source ? source.length + 1 : 0;
        HEAP32[((p)>>2)] = sourceLength;
      } else {
        HEAP32[((p)>>2)] = GLctx.getShaderParameter(GL.shaders[shader], pname);
      }
    };
  var _emscripten_glGetShaderiv = _glGetShaderiv;

  
  
  /** @suppress {duplicate } */
  var _glGetString = (name_) => {
      var ret = GL.stringCache[name_];
      if (!ret) {
        switch (name_) {
          case 0x1F03 /* GL_EXTENSIONS */:
            ret = stringToNewUTF8(webglGetExtensions().join(' '));
            break;
          case 0x1F00 /* GL_VENDOR */:
          case 0x1F01 /* GL_RENDERER */:
          case 0x9245 /* UNMASKED_VENDOR_WEBGL */:
          case 0x9246 /* UNMASKED_RENDERER_WEBGL */:
            var s = GLctx.getParameter(name_);
            if (!s) {
              GL.recordError(0x500/*GL_INVALID_ENUM*/);
            }
            ret = s ? stringToNewUTF8(s) : 0;
            break;
  
          case 0x1F02 /* GL_VERSION */:
            var webGLVersion = GLctx.getParameter(0x1F02 /*GL_VERSION*/);
            // return GLES version string corresponding to the version of the WebGL context
            var glVersion = `OpenGL ES 2.0 (${webGLVersion})`;
            if (true) glVersion = `OpenGL ES 3.0 (${webGLVersion})`;
            ret = stringToNewUTF8(glVersion);
            break;
          case 0x8B8C /* GL_SHADING_LANGUAGE_VERSION */:
            var glslVersion = GLctx.getParameter(0x8B8C /*GL_SHADING_LANGUAGE_VERSION*/);
            // extract the version number 'N.M' from the string 'WebGL GLSL ES N.M ...'
            var ver_re = /^WebGL GLSL ES ([0-9]\.[0-9][0-9]?)(?:$| .*)/;
            var ver_num = glslVersion.match(ver_re);
            if (ver_num !== null) {
              if (ver_num[1].length == 3) ver_num[1] = ver_num[1] + '0'; // ensure minor version has 2 digits
              glslVersion = `OpenGL ES GLSL ES ${ver_num[1]} (${glslVersion})`;
            }
            ret = stringToNewUTF8(glslVersion);
            break;
          default:
            GL.recordError(0x500/*GL_INVALID_ENUM*/);
            // fall through
        }
        GL.stringCache[name_] = ret;
      }
      return ret;
    };
  var _emscripten_glGetString = _glGetString;

  
  /** @suppress {duplicate } */
  var _glGetStringi = (name, index) => {
      if (GL.currentContext.version < 2) {
        GL.recordError(0x502 /* GL_INVALID_OPERATION */); // Calling GLES3/WebGL2 function with a GLES2/WebGL1 context
        return 0;
      }
      var stringiCache = GL.stringiCache[name];
      if (stringiCache) {
        if (index < 0 || index >= stringiCache.length) {
          GL.recordError(0x501/*GL_INVALID_VALUE*/);
          return 0;
        }
        return stringiCache[index];
      }
      switch (name) {
        case 0x1F03 /* GL_EXTENSIONS */:
          var exts = webglGetExtensions().map(stringToNewUTF8);
          stringiCache = GL.stringiCache[name] = exts;
          if (index < 0 || index >= stringiCache.length) {
            GL.recordError(0x501/*GL_INVALID_VALUE*/);
            return 0;
          }
          return stringiCache[index];
        default:
          GL.recordError(0x500/*GL_INVALID_ENUM*/);
          return 0;
      }
    };
  var _emscripten_glGetStringi = _glGetStringi;

  /** @suppress {duplicate } */
  var _glGetSynciv = (sync, pname, bufSize, length, values) => {
      if (bufSize < 0) {
        // GLES3 specification does not specify how to behave if bufSize < 0, however in the spec wording for glGetInternalformativ, it does say that GL_INVALID_VALUE should be raised,
        // so raise GL_INVALID_VALUE here as well.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      if (!values) {
        // GLES3 specification does not specify how to behave if values is a null pointer. Since calling this function does not make sense
        // if values == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var ret = GLctx.getSyncParameter(GL.syncs[sync], pname);
      if (ret !== null) {
        HEAP32[((values)>>2)] = ret;
        if (length) HEAP32[((length)>>2)] = 1; // Report a single value outputted.
      }
    };
  var _emscripten_glGetSynciv = _glGetSynciv;

  /** @suppress {duplicate } */
  var _glGetTexParameterfv = (target, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null
        // pointer. Since calling this function does not make sense if p == null,
        // issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAPF32[((params)>>2)] = GLctx.getTexParameter(target, pname);
    };
  var _emscripten_glGetTexParameterfv = _glGetTexParameterfv;

  /** @suppress {duplicate } */
  var _glGetTexParameteriv = (target, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null
        // pointer. Since calling this function does not make sense if p == null,
        // issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((params)>>2)] = GLctx.getTexParameter(target, pname);
    };
  var _emscripten_glGetTexParameteriv = _glGetTexParameteriv;

  /** @suppress {duplicate } */
  var _glGetTransformFeedbackVarying = (program, index, bufSize, length, size, type, name) => {
      program = GL.programs[program];
      var info = GLctx.getTransformFeedbackVarying(program, index);
      if (!info) return; // If an error occurred, the return parameters length, size, type and name will be unmodified.
  
      if (name && bufSize > 0) {
        var numBytesWrittenExclNull = stringToUTF8(info.name, name, bufSize);
        if (length) HEAP32[((length)>>2)] = numBytesWrittenExclNull;
      } else {
        if (length) HEAP32[((length)>>2)] = 0;
      }
  
      if (size) HEAP32[((size)>>2)] = info.size;
      if (type) HEAP32[((type)>>2)] = info.type;
    };
  var _emscripten_glGetTransformFeedbackVarying = _glGetTransformFeedbackVarying;

  /** @suppress {duplicate } */
  var _glGetUniformBlockIndex = (program, uniformBlockName) => {
      return GLctx.getUniformBlockIndex(GL.programs[program], UTF8ToString(uniformBlockName));
    };
  var _emscripten_glGetUniformBlockIndex = _glGetUniformBlockIndex;

  /** @suppress {duplicate } */
  var _glGetUniformIndices = (program, uniformCount, uniformNames, uniformIndices) => {
      if (!uniformIndices) {
        // GLES2 specification does not specify how to behave if uniformIndices is a null pointer. Since calling this function does not make sense
        // if uniformIndices == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      if (uniformCount > 0 && (uniformNames == 0 || uniformIndices == 0)) {
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      program = GL.programs[program];
      var names = [];
      for (var i = 0; i < uniformCount; i++)
        names.push(UTF8ToString(HEAP32[(((uniformNames)+(i*4))>>2)]));
  
      var result = GLctx.getUniformIndices(program, names);
      if (!result) return; // GL spec: If an error is generated, nothing is written out to uniformIndices.
  
      var len = result.length;
      for (var i = 0; i < len; i++) {
        HEAP32[(((uniformIndices)+(i*4))>>2)] = result[i];
      }
    };
  var _emscripten_glGetUniformIndices = _glGetUniformIndices;

  /** @suppress {checkTypes} */
  var jstoi_q = (str) => parseInt(str);
  
  /** @noinline */
  var webglGetLeftBracePos = (name) => name.slice(-1) == ']' && name.lastIndexOf('[');
  
  var webglPrepareUniformLocationsBeforeFirstUse = (program) => {
      var uniformLocsById = program.uniformLocsById, // Maps GLuint -> WebGLUniformLocation
        uniformSizeAndIdsByName = program.uniformSizeAndIdsByName, // Maps name -> [uniform array length, GLuint]
        i, j;
  
      // On the first time invocation of glGetUniformLocation on this shader program:
      // initialize cache data structures and discover which uniforms are arrays.
      if (!uniformLocsById) {
        // maps GLint integer locations to WebGLUniformLocations
        program.uniformLocsById = uniformLocsById = {};
        // maps integer locations back to uniform name strings, so that we can lazily fetch uniform array locations
        program.uniformArrayNamesById = {};
  
        var numActiveUniforms = GLctx.getProgramParameter(program, 0x8B86/*GL_ACTIVE_UNIFORMS*/);
        for (i = 0; i < numActiveUniforms; ++i) {
          var u = GLctx.getActiveUniform(program, i);
          var nm = u.name;
          var sz = u.size;
          var lb = webglGetLeftBracePos(nm);
          var arrayName = lb > 0 ? nm.slice(0, lb) : nm;
  
          // Assign a new location.
          var id = program.uniformIdCounter;
          program.uniformIdCounter += sz;
          // Eagerly get the location of the uniformArray[0] base element.
          // The remaining indices >0 will be left for lazy evaluation to
          // improve performance. Those may never be needed to fetch, if the
          // application fills arrays always in full starting from the first
          // element of the array.
          uniformSizeAndIdsByName[arrayName] = [sz, id];
  
          // Store placeholder integers in place that highlight that these
          // >0 index locations are array indices pending population.
          for (j = 0; j < sz; ++j) {
            uniformLocsById[id] = j;
            program.uniformArrayNamesById[id++] = arrayName;
          }
        }
      }
    };
  
  
  
  /** @suppress {duplicate } */
  var _glGetUniformLocation = (program, name) => {
  
      name = UTF8ToString(name);
  
      if (program = GL.programs[program]) {
        webglPrepareUniformLocationsBeforeFirstUse(program);
        var uniformLocsById = program.uniformLocsById; // Maps GLuint -> WebGLUniformLocation
        var arrayIndex = 0;
        var uniformBaseName = name;
  
        // Invariant: when populating integer IDs for uniform locations, we must
        // maintain the precondition that arrays reside in contiguous addresses,
        // i.e. for a 'vec4 colors[10];', colors[4] must be at location
        // colors[0]+4.  However, user might call glGetUniformLocation(program,
        // "colors") for an array, so we cannot discover based on the user input
        // arguments whether the uniform we are dealing with is an array. The only
        // way to discover which uniforms are arrays is to enumerate over all the
        // active uniforms in the program.
        var leftBrace = webglGetLeftBracePos(name);
  
        // If user passed an array accessor "[index]", parse the array index off the accessor.
        if (leftBrace > 0) {
          arrayIndex = jstoi_q(name.slice(leftBrace + 1)) >>> 0; // "index]", coerce parseInt(']') with >>>0 to treat "foo[]" as "foo[0]" and foo[-1] as unsigned out-of-bounds.
          uniformBaseName = name.slice(0, leftBrace);
        }
  
        // Have we cached the location of this uniform before?
        // A pair [array length, GLint of the uniform location]
        var sizeAndId = program.uniformSizeAndIdsByName[uniformBaseName];
  
        // If an uniform with this name exists, and if its index is within the
        // array limits (if it's even an array), query the WebGLlocation, or
        // return an existing cached location.
        if (sizeAndId && arrayIndex < sizeAndId[0]) {
          arrayIndex += sizeAndId[1]; // Add the base location of the uniform to the array index offset.
          if ((uniformLocsById[arrayIndex] = uniformLocsById[arrayIndex] || GLctx.getUniformLocation(program, name))) {
            return arrayIndex;
          }
        }
      }
      else {
        // N.b. we are currently unable to distinguish between GL program IDs that
        // never existed vs GL program IDs that have been deleted, so report
        // GL_INVALID_VALUE in both cases.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
      }
      return -1;
    };
  var _emscripten_glGetUniformLocation = _glGetUniformLocation;

  var webglGetUniformLocation = (location) => {
      var p = GLctx.currentProgram;
  
      if (p) {
        var webglLoc = p.uniformLocsById[location];
        // p.uniformLocsById[location] stores either an integer, or a
        // WebGLUniformLocation.
        // If an integer, we have not yet bound the location, so do it now. The
        // integer value specifies the array index we should bind to.
        if (typeof webglLoc == 'number') {
          p.uniformLocsById[location] = webglLoc = GLctx.getUniformLocation(p, p.uniformArrayNamesById[location] + (webglLoc > 0 ? `[${webglLoc}]` : ''));
        }
        // Else an already cached WebGLUniformLocation, return it.
        return webglLoc;
      } else {
        GL.recordError(0x502/*GL_INVALID_OPERATION*/);
      }
    };
  
  
  /** @suppress{checkTypes} */
  var emscriptenWebGLGetUniform = (program, location, params, type) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null
        // pointer. Since calling this function does not make sense if params ==
        // null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      program = GL.programs[program];
      webglPrepareUniformLocationsBeforeFirstUse(program);
      var data = GLctx.getUniform(program, webglGetUniformLocation(location));
      if (typeof data == 'number' || typeof data == 'boolean') {
        switch (type) {
          case 0: HEAP32[((params)>>2)] = data; break;
          case 2: HEAPF32[((params)>>2)] = data; break;
        }
      } else {
        for (var i = 0; i < data.length; i++) {
          switch (type) {
            case 0: HEAP32[(((params)+(i*4))>>2)] = data[i]; break;
            case 2: HEAPF32[(((params)+(i*4))>>2)] = data[i]; break;
          }
        }
      }
    };
  
  /** @suppress {duplicate } */
  var _glGetUniformfv = (program, location, params) => {
      emscriptenWebGLGetUniform(program, location, params, 2);
    };
  var _emscripten_glGetUniformfv = _glGetUniformfv;

  
  /** @suppress {duplicate } */
  var _glGetUniformiv = (program, location, params) => {
      emscriptenWebGLGetUniform(program, location, params, 0);
    };
  var _emscripten_glGetUniformiv = _glGetUniformiv;

  /** @suppress {duplicate } */
  var _glGetUniformuiv = (program, location, params) =>
      emscriptenWebGLGetUniform(program, location, params, 0);
  var _emscripten_glGetUniformuiv = _glGetUniformuiv;

  /** @suppress{checkTypes} */
  var emscriptenWebGLGetVertexAttrib = (index, pname, params, type) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null
        // pointer. Since calling this function does not make sense if params ==
        // null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var data = GLctx.getVertexAttrib(index, pname);
      if (pname == 0x889F/*VERTEX_ATTRIB_ARRAY_BUFFER_BINDING*/) {
        HEAP32[((params)>>2)] = data && data["name"];
      } else if (typeof data == 'number' || typeof data == 'boolean') {
        switch (type) {
          case 0: HEAP32[((params)>>2)] = data; break;
          case 2: HEAPF32[((params)>>2)] = data; break;
          case 5: HEAP32[((params)>>2)] = Math.fround(data); break;
        }
      } else {
        for (var i = 0; i < data.length; i++) {
          switch (type) {
            case 0: HEAP32[(((params)+(i*4))>>2)] = data[i]; break;
            case 2: HEAPF32[(((params)+(i*4))>>2)] = data[i]; break;
            case 5: HEAP32[(((params)+(i*4))>>2)] = Math.fround(data[i]); break;
          }
        }
      }
    };
  /** @suppress {duplicate } */
  var _glGetVertexAttribIiv = (index, pname, params) => {
      // N.B. This function may only be called if the vertex attribute was specified using the function glVertexAttribI4iv(),
      // otherwise the results are undefined. (GLES3 spec 6.1.12)
      emscriptenWebGLGetVertexAttrib(index, pname, params, 0);
    };
  var _emscripten_glGetVertexAttribIiv = _glGetVertexAttribIiv;

  
  /** @suppress {duplicate } */
  var _glGetVertexAttribIuiv = _glGetVertexAttribIiv;
  var _emscripten_glGetVertexAttribIuiv = _glGetVertexAttribIuiv;

  /** @suppress {duplicate } */
  var _glGetVertexAttribPointerv = (index, pname, pointer) => {
      if (!pointer) {
        // GLES2 specification does not specify how to behave if pointer is a null
        // pointer. Since calling this function does not make sense if pointer ==
        // null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((pointer)>>2)] = GLctx.getVertexAttribOffset(index, pname);
    };
  var _emscripten_glGetVertexAttribPointerv = _glGetVertexAttribPointerv;

  
  /** @suppress {duplicate } */
  var _glGetVertexAttribfv = (index, pname, params) => {
      // N.B. This function may only be called if the vertex attribute was
      // specified using the function glVertexAttrib*f(), otherwise the results
      // are undefined. (GLES3 spec 6.1.12)
      emscriptenWebGLGetVertexAttrib(index, pname, params, 2);
    };
  var _emscripten_glGetVertexAttribfv = _glGetVertexAttribfv;

  
  /** @suppress {duplicate } */
  var _glGetVertexAttribiv = (index, pname, params) => {
      // N.B. This function may only be called if the vertex attribute was
      // specified using the function glVertexAttrib*f(), otherwise the results
      // are undefined. (GLES3 spec 6.1.12)
      emscriptenWebGLGetVertexAttrib(index, pname, params, 5);
    };
  var _emscripten_glGetVertexAttribiv = _glGetVertexAttribiv;

  /** @suppress {duplicate } */
  var _glHint = (x0, x1) => GLctx.hint(x0, x1);
  var _emscripten_glHint = _glHint;

  /** @suppress {duplicate } */
  var _glInvalidateFramebuffer = (target, numAttachments, attachments) => {
      var list = tempFixedLengthArray[numAttachments];
      for (var i = 0; i < numAttachments; i++) {
        list[i] = HEAP32[(((attachments)+(i*4))>>2)];
      }
  
      GLctx.invalidateFramebuffer(target, list);
    };
  var _emscripten_glInvalidateFramebuffer = _glInvalidateFramebuffer;

  /** @suppress {duplicate } */
  var _glInvalidateSubFramebuffer = (target, numAttachments, attachments, x, y, width, height) => {
      var list = tempFixedLengthArray[numAttachments];
      for (var i = 0; i < numAttachments; i++) {
        list[i] = HEAP32[(((attachments)+(i*4))>>2)];
      }
  
      GLctx.invalidateSubFramebuffer(target, list, x, y, width, height);
    };
  var _emscripten_glInvalidateSubFramebuffer = _glInvalidateSubFramebuffer;

  /** @suppress {duplicate } */
  var _glIsBuffer = (buffer) => {
      var b = GL.buffers[buffer];
      if (!b) return 0;
      return GLctx.isBuffer(b);
    };
  var _emscripten_glIsBuffer = _glIsBuffer;

  /** @suppress {duplicate } */
  var _glIsEnabled = (x0) => GLctx.isEnabled(x0);
  var _emscripten_glIsEnabled = _glIsEnabled;

  /** @suppress {duplicate } */
  var _glIsFramebuffer = (framebuffer) => {
      var fb = GL.framebuffers[framebuffer];
      if (!fb) return 0;
      return GLctx.isFramebuffer(fb);
    };
  var _emscripten_glIsFramebuffer = _glIsFramebuffer;

  /** @suppress {duplicate } */
  var _glIsProgram = (program) => {
      program = GL.programs[program];
      if (!program) return 0;
      return GLctx.isProgram(program);
    };
  var _emscripten_glIsProgram = _glIsProgram;

  /** @suppress {duplicate } */
  var _glIsQuery = (id) => {
      var query = GL.queries[id];
      if (!query) return 0;
      return GLctx.isQuery(query);
    };
  var _emscripten_glIsQuery = _glIsQuery;

  /** @suppress {duplicate } */
  var _glIsQueryEXT = (id) => {
      var query = GL.queries[id];
      if (!query) return 0;
      return GLctx.disjointTimerQueryExt['isQueryEXT'](query);
    };
  var _emscripten_glIsQueryEXT = _glIsQueryEXT;

  /** @suppress {duplicate } */
  var _glIsRenderbuffer = (renderbuffer) => {
      var rb = GL.renderbuffers[renderbuffer];
      if (!rb) return 0;
      return GLctx.isRenderbuffer(rb);
    };
  var _emscripten_glIsRenderbuffer = _glIsRenderbuffer;

  /** @suppress {duplicate } */
  var _glIsSampler = (id) => {
      var sampler = GL.samplers[id];
      if (!sampler) return 0;
      return GLctx.isSampler(sampler);
    };
  var _emscripten_glIsSampler = _glIsSampler;

  /** @suppress {duplicate } */
  var _glIsShader = (shader) => {
      var s = GL.shaders[shader];
      if (!s) return 0;
      return GLctx.isShader(s);
    };
  var _emscripten_glIsShader = _glIsShader;

  /** @suppress {duplicate } */
  var _glIsSync = (sync) => GLctx.isSync(GL.syncs[sync]);
  var _emscripten_glIsSync = _glIsSync;

  /** @suppress {duplicate } */
  var _glIsTexture = (id) => {
      var texture = GL.textures[id];
      if (!texture) return 0;
      return GLctx.isTexture(texture);
    };
  var _emscripten_glIsTexture = _glIsTexture;

  /** @suppress {duplicate } */
  var _glIsTransformFeedback = (id) => GLctx.isTransformFeedback(GL.transformFeedbacks[id]);
  var _emscripten_glIsTransformFeedback = _glIsTransformFeedback;

  /** @suppress {duplicate } */
  var _glIsVertexArray = (array) => {
  
      var vao = GL.vaos[array];
      if (!vao) return 0;
      return GLctx.isVertexArray(vao);
    };
  var _emscripten_glIsVertexArray = _glIsVertexArray;

  
  /** @suppress {duplicate } */
  var _glIsVertexArrayOES = _glIsVertexArray;
  var _emscripten_glIsVertexArrayOES = _glIsVertexArrayOES;

  /** @suppress {duplicate } */
  var _glLineWidth = (x0) => GLctx.lineWidth(x0);
  var _emscripten_glLineWidth = _glLineWidth;

  /** @suppress {duplicate } */
  var _glLinkProgram = (program) => {
      program = GL.programs[program];
      GLctx.linkProgram(program);
      // Invalidate earlier computed uniform->ID mappings, those have now become stale
      program.uniformLocsById = 0; // Mark as null-like so that glGetUniformLocation() knows to populate this again.
      program.uniformSizeAndIdsByName = {};
  
    };
  var _emscripten_glLinkProgram = _glLinkProgram;

  /** @suppress {duplicate } */
  var _glPauseTransformFeedback = () => GLctx.pauseTransformFeedback();
  var _emscripten_glPauseTransformFeedback = _glPauseTransformFeedback;

  /** @suppress {duplicate } */
  var _glPixelStorei = (pname, param) => {
      if (pname == 3317) {
        GL.unpackAlignment = param;
      } else if (pname == 3314) {
        GL.unpackRowLength = param;
      }
      GLctx.pixelStorei(pname, param);
    };
  var _emscripten_glPixelStorei = _glPixelStorei;

  /** @suppress {duplicate } */
  var _glPolygonModeWEBGL = (face, mode) => {
      GLctx.webglPolygonMode['polygonModeWEBGL'](face, mode);
    };
  var _emscripten_glPolygonModeWEBGL = _glPolygonModeWEBGL;

  /** @suppress {duplicate } */
  var _glPolygonOffset = (x0, x1) => GLctx.polygonOffset(x0, x1);
  var _emscripten_glPolygonOffset = _glPolygonOffset;

  /** @suppress {duplicate } */
  var _glPolygonOffsetClampEXT = (factor, units, clamp) => {
      GLctx.extPolygonOffsetClamp['polygonOffsetClampEXT'](factor, units, clamp);
    };
  var _emscripten_glPolygonOffsetClampEXT = _glPolygonOffsetClampEXT;

  /** @suppress {duplicate } */
  var _glProgramBinary = (program, binaryFormat, binary, length) => {
      GL.recordError(0x500/*GL_INVALID_ENUM*/);
    };
  var _emscripten_glProgramBinary = _glProgramBinary;

  /** @suppress {duplicate } */
  var _glProgramParameteri = (program, pname, value) => {
      GL.recordError(0x500/*GL_INVALID_ENUM*/);
    };
  var _emscripten_glProgramParameteri = _glProgramParameteri;

  /** @suppress {duplicate } */
  var _glQueryCounterEXT = (id, target) => {
      GLctx.disjointTimerQueryExt['queryCounterEXT'](GL.queries[id], target);
    };
  var _emscripten_glQueryCounterEXT = _glQueryCounterEXT;

  /** @suppress {duplicate } */
  var _glReadBuffer = (x0) => GLctx.readBuffer(x0);
  var _emscripten_glReadBuffer = _glReadBuffer;

  var heapObjectForWebGLType = (type) => {
      // Micro-optimization for size: Subtract lowest GL enum number (0x1400/* GL_BYTE */) from type to compare
      // smaller values for the heap, for shorter generated code size.
      // Also the type HEAPU16 is not tested for explicitly, but any unrecognized type will return out HEAPU16.
      // (since most types are HEAPU16)
      type -= 0x1400;
      if (type == 0) return HEAP8;
  
      if (type == 1) return HEAPU8;
  
      if (type == 2) return HEAP16;
  
      if (type == 4) return HEAP32;
  
      if (type == 6) return HEAPF32;
  
      if (type == 5
        || type == 28922
        || type == 28520
        || type == 30779
        || type == 30782
        )
        return HEAPU32;
  
      return HEAPU16;
    };
  
  var toTypedArrayIndex = (pointer, heap) =>
      pointer >>> (31 - Math.clz32(heap.BYTES_PER_ELEMENT));
  
  /** @suppress {duplicate } */
  var _glReadPixels = (x, y, width, height, format, type, pixels) => {
      if (true) {
        if (GLctx.currentPixelPackBufferBinding) {
          GLctx.readPixels(x, y, width, height, format, type, pixels);
          return;
        }
        var heap = heapObjectForWebGLType(type);
        var target = toTypedArrayIndex(pixels, heap);
        GLctx.readPixels(x, y, width, height, format, type, heap, target);
        return;
      }
    };
  var _emscripten_glReadPixels = _glReadPixels;

  /** @suppress {duplicate } */
  var _glReleaseShaderCompiler = () => {
      // NOP (as allowed by GLES 2.0 spec)
    };
  var _emscripten_glReleaseShaderCompiler = _glReleaseShaderCompiler;

  /** @suppress {duplicate } */
  var _glRenderbufferStorage = (x0, x1, x2, x3) => GLctx.renderbufferStorage(x0, x1, x2, x3);
  var _emscripten_glRenderbufferStorage = _glRenderbufferStorage;

  /** @suppress {duplicate } */
  var _glRenderbufferStorageMultisample = (x0, x1, x2, x3, x4) => GLctx.renderbufferStorageMultisample(x0, x1, x2, x3, x4);
  var _emscripten_glRenderbufferStorageMultisample = _glRenderbufferStorageMultisample;

  /** @suppress {duplicate } */
  var _glResumeTransformFeedback = () => GLctx.resumeTransformFeedback();
  var _emscripten_glResumeTransformFeedback = _glResumeTransformFeedback;

  /** @suppress {duplicate } */
  var _glSampleCoverage = (value, invert) => {
      GLctx.sampleCoverage(value, !!invert);
    };
  var _emscripten_glSampleCoverage = _glSampleCoverage;

  /** @suppress {duplicate } */
  var _glSamplerParameterf = (sampler, pname, param) => {
      GLctx.samplerParameterf(GL.samplers[sampler], pname, param);
    };
  var _emscripten_glSamplerParameterf = _glSamplerParameterf;

  /** @suppress {duplicate } */
  var _glSamplerParameterfv = (sampler, pname, params) => {
      var param = HEAPF32[((params)>>2)];
      GLctx.samplerParameterf(GL.samplers[sampler], pname, param);
    };
  var _emscripten_glSamplerParameterfv = _glSamplerParameterfv;

  /** @suppress {duplicate } */
  var _glSamplerParameteri = (sampler, pname, param) => {
      GLctx.samplerParameteri(GL.samplers[sampler], pname, param);
    };
  var _emscripten_glSamplerParameteri = _glSamplerParameteri;

  /** @suppress {duplicate } */
  var _glSamplerParameteriv = (sampler, pname, params) => {
      var param = HEAP32[((params)>>2)];
      GLctx.samplerParameteri(GL.samplers[sampler], pname, param);
    };
  var _emscripten_glSamplerParameteriv = _glSamplerParameteriv;

  /** @suppress {duplicate } */
  var _glScissor = (x0, x1, x2, x3) => GLctx.scissor(x0, x1, x2, x3);
  var _emscripten_glScissor = _glScissor;

  /** @suppress {duplicate } */
  var _glShaderBinary = (count, shaders, binaryformat, binary, length) => {
      GL.recordError(0x500/*GL_INVALID_ENUM*/);
    };
  var _emscripten_glShaderBinary = _glShaderBinary;

  /** @suppress {duplicate } */
  var _glShaderSource = (shader, count, string, length) => {
      var source = GL.getSource(shader, count, string, length);
  
      GLctx.shaderSource(GL.shaders[shader], source);
    };
  var _emscripten_glShaderSource = _glShaderSource;

  /** @suppress {duplicate } */
  var _glStencilFunc = (x0, x1, x2) => GLctx.stencilFunc(x0, x1, x2);
  var _emscripten_glStencilFunc = _glStencilFunc;

  /** @suppress {duplicate } */
  var _glStencilFuncSeparate = (x0, x1, x2, x3) => GLctx.stencilFuncSeparate(x0, x1, x2, x3);
  var _emscripten_glStencilFuncSeparate = _glStencilFuncSeparate;

  /** @suppress {duplicate } */
  var _glStencilMask = (x0) => GLctx.stencilMask(x0);
  var _emscripten_glStencilMask = _glStencilMask;

  /** @suppress {duplicate } */
  var _glStencilMaskSeparate = (x0, x1) => GLctx.stencilMaskSeparate(x0, x1);
  var _emscripten_glStencilMaskSeparate = _glStencilMaskSeparate;

  /** @suppress {duplicate } */
  var _glStencilOp = (x0, x1, x2) => GLctx.stencilOp(x0, x1, x2);
  var _emscripten_glStencilOp = _glStencilOp;

  /** @suppress {duplicate } */
  var _glStencilOpSeparate = (x0, x1, x2, x3) => GLctx.stencilOpSeparate(x0, x1, x2, x3);
  var _emscripten_glStencilOpSeparate = _glStencilOpSeparate;

  var computeUnpackAlignedImageSize = (width, height, sizePerPixel) => {
      function roundedToNextMultipleOf(x, y) {
        return (x + y - 1) & -y;
      }
      var plainRowSize = (GL.unpackRowLength || width) * sizePerPixel;
      var alignedRowSize = roundedToNextMultipleOf(plainRowSize, GL.unpackAlignment);
      return height * alignedRowSize;
    };
  
  var colorChannelsInGlTextureFormat = (format) => {
      // Micro-optimizations for size: map format to size by subtracting smallest
      // enum value (0x1902) from all values first.  Also omit the most common
      // size value (1) from the list, which is assumed by formats not on the
      // list.
      var colorChannels = {
        // 0x1902 /* GL_DEPTH_COMPONENT */ - 0x1902: 1,
        // 0x1906 /* GL_ALPHA */ - 0x1902: 1,
        5: 3,
        6: 4,
        // 0x1909 /* GL_LUMINANCE */ - 0x1902: 1,
        8: 2,
        29502: 3,
        29504: 4,
        // 0x1903 /* GL_RED */ - 0x1902: 1,
        26917: 2,
        26918: 2,
        // 0x8D94 /* GL_RED_INTEGER */ - 0x1902: 1,
        29846: 3,
        29847: 4
      };
      return colorChannels[format - 0x1902]||1;
    };
  
  
  
  var emscriptenWebGLGetTexPixelData = (type, format, width, height, pixels, internalFormat) => {
      var heap = heapObjectForWebGLType(type);
      var sizePerPixel = colorChannelsInGlTextureFormat(format) * heap.BYTES_PER_ELEMENT;
      var bytes = computeUnpackAlignedImageSize(width, height, sizePerPixel);
      return heap.subarray(toTypedArrayIndex(pixels, heap), toTypedArrayIndex(pixels + bytes, heap));
    };
  
  
  
  /** @suppress {duplicate } */
  var _glTexImage2D = (target, level, internalFormat, width, height, border, format, type, pixels) => {
      if (true) {
        if (GLctx.currentPixelUnpackBufferBinding) {
          GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
          return;
        }
        if (pixels) {
          var heap = heapObjectForWebGLType(type);
          var index = toTypedArrayIndex(pixels, heap);
          GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, heap, index);
          return;
        }
      }
      var pixelData = pixels ? emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, internalFormat) : null;
      GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixelData);
    };
  var _emscripten_glTexImage2D = _glTexImage2D;

  
  /** @suppress {duplicate } */
  var _glTexImage3D = (target, level, internalFormat, width, height, depth, border, format, type, pixels) => {
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx.texImage3D(target, level, internalFormat, width, height, depth, border, format, type, pixels);
      } else if (pixels) {
        var heap = heapObjectForWebGLType(type);
        GLctx.texImage3D(target, level, internalFormat, width, height, depth, border, format, type, heap, toTypedArrayIndex(pixels, heap));
      } else {
        GLctx.texImage3D(target, level, internalFormat, width, height, depth, border, format, type, null);
      }
    };
  var _emscripten_glTexImage3D = _glTexImage3D;

  /** @suppress {duplicate } */
  var _glTexParameterf = (x0, x1, x2) => GLctx.texParameterf(x0, x1, x2);
  var _emscripten_glTexParameterf = _glTexParameterf;

  /** @suppress {duplicate } */
  var _glTexParameterfv = (target, pname, params) => {
      var param = HEAPF32[((params)>>2)];
      GLctx.texParameterf(target, pname, param);
    };
  var _emscripten_glTexParameterfv = _glTexParameterfv;

  /** @suppress {duplicate } */
  var _glTexParameteri = (x0, x1, x2) => GLctx.texParameteri(x0, x1, x2);
  var _emscripten_glTexParameteri = _glTexParameteri;

  /** @suppress {duplicate } */
  var _glTexParameteriv = (target, pname, params) => {
      var param = HEAP32[((params)>>2)];
      GLctx.texParameteri(target, pname, param);
    };
  var _emscripten_glTexParameteriv = _glTexParameteriv;

  /** @suppress {duplicate } */
  var _glTexStorage2D = (x0, x1, x2, x3, x4) => GLctx.texStorage2D(x0, x1, x2, x3, x4);
  var _emscripten_glTexStorage2D = _glTexStorage2D;

  /** @suppress {duplicate } */
  var _glTexStorage3D = (x0, x1, x2, x3, x4, x5) => GLctx.texStorage3D(x0, x1, x2, x3, x4, x5);
  var _emscripten_glTexStorage3D = _glTexStorage3D;

  
  
  
  /** @suppress {duplicate } */
  var _glTexSubImage2D = (target, level, xoffset, yoffset, width, height, format, type, pixels) => {
      if (true) {
        if (GLctx.currentPixelUnpackBufferBinding) {
          GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
          return;
        }
        if (pixels) {
          var heap = heapObjectForWebGLType(type);
          GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, heap, toTypedArrayIndex(pixels, heap));
          return;
        }
      }
      var pixelData = pixels ? emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, 0) : null;
      GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixelData);
    };
  var _emscripten_glTexSubImage2D = _glTexSubImage2D;

  
  /** @suppress {duplicate } */
  var _glTexSubImage3D = (target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) => {
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx.texSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
      } else if (pixels) {
        var heap = heapObjectForWebGLType(type);
        GLctx.texSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, heap, toTypedArrayIndex(pixels, heap));
      } else {
        GLctx.texSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, null);
      }
    };
  var _emscripten_glTexSubImage3D = _glTexSubImage3D;

  /** @suppress {duplicate } */
  var _glTransformFeedbackVaryings = (program, count, varyings, bufferMode) => {
      program = GL.programs[program];
      var vars = [];
      for (var i = 0; i < count; i++)
        vars.push(UTF8ToString(HEAP32[(((varyings)+(i*4))>>2)]));
  
      GLctx.transformFeedbackVaryings(program, vars, bufferMode);
    };
  var _emscripten_glTransformFeedbackVaryings = _glTransformFeedbackVaryings;

  
  /** @suppress {duplicate } */
  var _glUniform1f = (location, v0) => {
      GLctx.uniform1f(webglGetUniformLocation(location), v0);
    };
  var _emscripten_glUniform1f = _glUniform1f;

  
  /** @suppress {duplicate } */
  var _glUniform1fv = (location, count, value) => {
  
      count && GLctx.uniform1fv(webglGetUniformLocation(location), HEAPF32, ((value)>>2), count);
    };
  var _emscripten_glUniform1fv = _glUniform1fv;

  
  /** @suppress {duplicate } */
  var _glUniform1i = (location, v0) => {
      GLctx.uniform1i(webglGetUniformLocation(location), v0);
    };
  var _emscripten_glUniform1i = _glUniform1i;

  
  /** @suppress {duplicate } */
  var _glUniform1iv = (location, count, value) => {
  
      count && GLctx.uniform1iv(webglGetUniformLocation(location), HEAP32, ((value)>>2), count);
    };
  var _emscripten_glUniform1iv = _glUniform1iv;

  /** @suppress {duplicate } */
  var _glUniform1ui = (location, v0) => {
      GLctx.uniform1ui(webglGetUniformLocation(location), v0);
    };
  var _emscripten_glUniform1ui = _glUniform1ui;

  /** @suppress {duplicate } */
  var _glUniform1uiv = (location, count, value) => {
      count && GLctx.uniform1uiv(webglGetUniformLocation(location), HEAPU32, ((value)>>2), count);
    };
  var _emscripten_glUniform1uiv = _glUniform1uiv;

  
  /** @suppress {duplicate } */
  var _glUniform2f = (location, v0, v1) => {
      GLctx.uniform2f(webglGetUniformLocation(location), v0, v1);
    };
  var _emscripten_glUniform2f = _glUniform2f;

  
  /** @suppress {duplicate } */
  var _glUniform2fv = (location, count, value) => {
  
      count && GLctx.uniform2fv(webglGetUniformLocation(location), HEAPF32, ((value)>>2), count*2);
    };
  var _emscripten_glUniform2fv = _glUniform2fv;

  
  /** @suppress {duplicate } */
  var _glUniform2i = (location, v0, v1) => {
      GLctx.uniform2i(webglGetUniformLocation(location), v0, v1);
    };
  var _emscripten_glUniform2i = _glUniform2i;

  
  /** @suppress {duplicate } */
  var _glUniform2iv = (location, count, value) => {
  
      count && GLctx.uniform2iv(webglGetUniformLocation(location), HEAP32, ((value)>>2), count*2);
    };
  var _emscripten_glUniform2iv = _glUniform2iv;

  /** @suppress {duplicate } */
  var _glUniform2ui = (location, v0, v1) => {
      GLctx.uniform2ui(webglGetUniformLocation(location), v0, v1);
    };
  var _emscripten_glUniform2ui = _glUniform2ui;

  /** @suppress {duplicate } */
  var _glUniform2uiv = (location, count, value) => {
      count && GLctx.uniform2uiv(webglGetUniformLocation(location), HEAPU32, ((value)>>2), count*2);
    };
  var _emscripten_glUniform2uiv = _glUniform2uiv;

  
  /** @suppress {duplicate } */
  var _glUniform3f = (location, v0, v1, v2) => {
      GLctx.uniform3f(webglGetUniformLocation(location), v0, v1, v2);
    };
  var _emscripten_glUniform3f = _glUniform3f;

  
  /** @suppress {duplicate } */
  var _glUniform3fv = (location, count, value) => {
  
      count && GLctx.uniform3fv(webglGetUniformLocation(location), HEAPF32, ((value)>>2), count*3);
    };
  var _emscripten_glUniform3fv = _glUniform3fv;

  
  /** @suppress {duplicate } */
  var _glUniform3i = (location, v0, v1, v2) => {
      GLctx.uniform3i(webglGetUniformLocation(location), v0, v1, v2);
    };
  var _emscripten_glUniform3i = _glUniform3i;

  
  /** @suppress {duplicate } */
  var _glUniform3iv = (location, count, value) => {
  
      count && GLctx.uniform3iv(webglGetUniformLocation(location), HEAP32, ((value)>>2), count*3);
    };
  var _emscripten_glUniform3iv = _glUniform3iv;

  /** @suppress {duplicate } */
  var _glUniform3ui = (location, v0, v1, v2) => {
      GLctx.uniform3ui(webglGetUniformLocation(location), v0, v1, v2);
    };
  var _emscripten_glUniform3ui = _glUniform3ui;

  /** @suppress {duplicate } */
  var _glUniform3uiv = (location, count, value) => {
      count && GLctx.uniform3uiv(webglGetUniformLocation(location), HEAPU32, ((value)>>2), count*3);
    };
  var _emscripten_glUniform3uiv = _glUniform3uiv;

  
  /** @suppress {duplicate } */
  var _glUniform4f = (location, v0, v1, v2, v3) => {
      GLctx.uniform4f(webglGetUniformLocation(location), v0, v1, v2, v3);
    };
  var _emscripten_glUniform4f = _glUniform4f;

  
  /** @suppress {duplicate } */
  var _glUniform4fv = (location, count, value) => {
  
      count && GLctx.uniform4fv(webglGetUniformLocation(location), HEAPF32, ((value)>>2), count*4);
    };
  var _emscripten_glUniform4fv = _glUniform4fv;

  
  /** @suppress {duplicate } */
  var _glUniform4i = (location, v0, v1, v2, v3) => {
      GLctx.uniform4i(webglGetUniformLocation(location), v0, v1, v2, v3);
    };
  var _emscripten_glUniform4i = _glUniform4i;

  
  /** @suppress {duplicate } */
  var _glUniform4iv = (location, count, value) => {
  
      count && GLctx.uniform4iv(webglGetUniformLocation(location), HEAP32, ((value)>>2), count*4);
    };
  var _emscripten_glUniform4iv = _glUniform4iv;

  /** @suppress {duplicate } */
  var _glUniform4ui = (location, v0, v1, v2, v3) => {
      GLctx.uniform4ui(webglGetUniformLocation(location), v0, v1, v2, v3);
    };
  var _emscripten_glUniform4ui = _glUniform4ui;

  /** @suppress {duplicate } */
  var _glUniform4uiv = (location, count, value) => {
      count && GLctx.uniform4uiv(webglGetUniformLocation(location), HEAPU32, ((value)>>2), count*4);
    };
  var _emscripten_glUniform4uiv = _glUniform4uiv;

  /** @suppress {duplicate } */
  var _glUniformBlockBinding = (program, uniformBlockIndex, uniformBlockBinding) => {
      program = GL.programs[program];
  
      GLctx.uniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
    };
  var _emscripten_glUniformBlockBinding = _glUniformBlockBinding;

  
  /** @suppress {duplicate } */
  var _glUniformMatrix2fv = (location, count, transpose, value) => {
  
      count && GLctx.uniformMatrix2fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*4);
    };
  var _emscripten_glUniformMatrix2fv = _glUniformMatrix2fv;

  /** @suppress {duplicate } */
  var _glUniformMatrix2x3fv = (location, count, transpose, value) => {
      count && GLctx.uniformMatrix2x3fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*6);
    };
  var _emscripten_glUniformMatrix2x3fv = _glUniformMatrix2x3fv;

  /** @suppress {duplicate } */
  var _glUniformMatrix2x4fv = (location, count, transpose, value) => {
      count && GLctx.uniformMatrix2x4fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*8);
    };
  var _emscripten_glUniformMatrix2x4fv = _glUniformMatrix2x4fv;

  
  /** @suppress {duplicate } */
  var _glUniformMatrix3fv = (location, count, transpose, value) => {
  
      count && GLctx.uniformMatrix3fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*9);
    };
  var _emscripten_glUniformMatrix3fv = _glUniformMatrix3fv;

  /** @suppress {duplicate } */
  var _glUniformMatrix3x2fv = (location, count, transpose, value) => {
      count && GLctx.uniformMatrix3x2fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*6);
    };
  var _emscripten_glUniformMatrix3x2fv = _glUniformMatrix3x2fv;

  /** @suppress {duplicate } */
  var _glUniformMatrix3x4fv = (location, count, transpose, value) => {
      count && GLctx.uniformMatrix3x4fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*12);
    };
  var _emscripten_glUniformMatrix3x4fv = _glUniformMatrix3x4fv;

  
  /** @suppress {duplicate } */
  var _glUniformMatrix4fv = (location, count, transpose, value) => {
  
      count && GLctx.uniformMatrix4fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*16);
    };
  var _emscripten_glUniformMatrix4fv = _glUniformMatrix4fv;

  /** @suppress {duplicate } */
  var _glUniformMatrix4x2fv = (location, count, transpose, value) => {
      count && GLctx.uniformMatrix4x2fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*8);
    };
  var _emscripten_glUniformMatrix4x2fv = _glUniformMatrix4x2fv;

  /** @suppress {duplicate } */
  var _glUniformMatrix4x3fv = (location, count, transpose, value) => {
      count && GLctx.uniformMatrix4x3fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*12);
    };
  var _emscripten_glUniformMatrix4x3fv = _glUniformMatrix4x3fv;

  /** @suppress {duplicate } */
  var _glUseProgram = (program) => {
      program = GL.programs[program];
      GLctx.useProgram(program);
      // Record the currently active program so that we can access the uniform
      // mapping table of that program.
      GLctx.currentProgram = program;
    };
  var _emscripten_glUseProgram = _glUseProgram;

  /** @suppress {duplicate } */
  var _glValidateProgram = (program) => {
      GLctx.validateProgram(GL.programs[program]);
    };
  var _emscripten_glValidateProgram = _glValidateProgram;

  /** @suppress {duplicate } */
  var _glVertexAttrib1f = (x0, x1) => GLctx.vertexAttrib1f(x0, x1);
  var _emscripten_glVertexAttrib1f = _glVertexAttrib1f;

  /** @suppress {duplicate } */
  var _glVertexAttrib1fv = (index, v) => {
  
      GLctx.vertexAttrib1f(index, HEAPF32[v>>2]);
    };
  var _emscripten_glVertexAttrib1fv = _glVertexAttrib1fv;

  /** @suppress {duplicate } */
  var _glVertexAttrib2f = (x0, x1, x2) => GLctx.vertexAttrib2f(x0, x1, x2);
  var _emscripten_glVertexAttrib2f = _glVertexAttrib2f;

  /** @suppress {duplicate } */
  var _glVertexAttrib2fv = (index, v) => {
  
      GLctx.vertexAttrib2f(index, HEAPF32[v>>2], HEAPF32[v+4>>2]);
    };
  var _emscripten_glVertexAttrib2fv = _glVertexAttrib2fv;

  /** @suppress {duplicate } */
  var _glVertexAttrib3f = (x0, x1, x2, x3) => GLctx.vertexAttrib3f(x0, x1, x2, x3);
  var _emscripten_glVertexAttrib3f = _glVertexAttrib3f;

  /** @suppress {duplicate } */
  var _glVertexAttrib3fv = (index, v) => {
  
      GLctx.vertexAttrib3f(index, HEAPF32[v>>2], HEAPF32[v+4>>2], HEAPF32[v+8>>2]);
    };
  var _emscripten_glVertexAttrib3fv = _glVertexAttrib3fv;

  /** @suppress {duplicate } */
  var _glVertexAttrib4f = (x0, x1, x2, x3, x4) => GLctx.vertexAttrib4f(x0, x1, x2, x3, x4);
  var _emscripten_glVertexAttrib4f = _glVertexAttrib4f;

  /** @suppress {duplicate } */
  var _glVertexAttrib4fv = (index, v) => {
  
      GLctx.vertexAttrib4f(index, HEAPF32[v>>2], HEAPF32[v+4>>2], HEAPF32[v+8>>2], HEAPF32[v+12>>2]);
    };
  var _emscripten_glVertexAttrib4fv = _glVertexAttrib4fv;

  /** @suppress {duplicate } */
  var _glVertexAttribDivisor = (index, divisor) => {
      GLctx.vertexAttribDivisor(index, divisor);
    };
  var _emscripten_glVertexAttribDivisor = _glVertexAttribDivisor;

  
  /** @suppress {duplicate } */
  var _glVertexAttribDivisorANGLE = _glVertexAttribDivisor;
  var _emscripten_glVertexAttribDivisorANGLE = _glVertexAttribDivisorANGLE;

  
  /** @suppress {duplicate } */
  var _glVertexAttribDivisorARB = _glVertexAttribDivisor;
  var _emscripten_glVertexAttribDivisorARB = _glVertexAttribDivisorARB;

  
  /** @suppress {duplicate } */
  var _glVertexAttribDivisorEXT = _glVertexAttribDivisor;
  var _emscripten_glVertexAttribDivisorEXT = _glVertexAttribDivisorEXT;

  
  /** @suppress {duplicate } */
  var _glVertexAttribDivisorNV = _glVertexAttribDivisor;
  var _emscripten_glVertexAttribDivisorNV = _glVertexAttribDivisorNV;

  /** @suppress {duplicate } */
  var _glVertexAttribI4i = (x0, x1, x2, x3, x4) => GLctx.vertexAttribI4i(x0, x1, x2, x3, x4);
  var _emscripten_glVertexAttribI4i = _glVertexAttribI4i;

  /** @suppress {duplicate } */
  var _glVertexAttribI4iv = (index, v) => {
      GLctx.vertexAttribI4i(index, HEAP32[v>>2], HEAP32[v+4>>2], HEAP32[v+8>>2], HEAP32[v+12>>2]);
    };
  var _emscripten_glVertexAttribI4iv = _glVertexAttribI4iv;

  /** @suppress {duplicate } */
  var _glVertexAttribI4ui = (x0, x1, x2, x3, x4) => GLctx.vertexAttribI4ui(x0, x1, x2, x3, x4);
  var _emscripten_glVertexAttribI4ui = _glVertexAttribI4ui;

  /** @suppress {duplicate } */
  var _glVertexAttribI4uiv = (index, v) => {
      GLctx.vertexAttribI4ui(index, HEAPU32[v>>2], HEAPU32[v+4>>2], HEAPU32[v+8>>2], HEAPU32[v+12>>2]);
    };
  var _emscripten_glVertexAttribI4uiv = _glVertexAttribI4uiv;

  /** @suppress {duplicate } */
  var _glVertexAttribIPointer = (index, size, type, stride, ptr) => {
      GLctx.vertexAttribIPointer(index, size, type, stride, ptr);
    };
  var _emscripten_glVertexAttribIPointer = _glVertexAttribIPointer;

  /** @suppress {duplicate } */
  var _glVertexAttribPointer = (index, size, type, normalized, stride, ptr) => {
      GLctx.vertexAttribPointer(index, size, type, !!normalized, stride, ptr);
    };
  var _emscripten_glVertexAttribPointer = _glVertexAttribPointer;

  /** @suppress {duplicate } */
  var _glViewport = (x0, x1, x2, x3) => GLctx.viewport(x0, x1, x2, x3);
  var _emscripten_glViewport = _glViewport;

  /** @suppress {duplicate } */
  var _glWaitSync = (sync, flags, timeout) => {
      // See WebGL2 vs GLES3 difference on GL_TIMEOUT_IGNORED above (https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.15)
      timeout = Number(timeout);
      GLctx.waitSync(GL.syncs[sync], flags, timeout);
    };
  var _emscripten_glWaitSync = _glWaitSync;

  var _emscripten_has_asyncify = () => 0;

  var _emscripten_is_main_browser_thread = () =>
      !ENVIRONMENT_IS_WORKER;

  var reallyNegative = (x) => x < 0 || (x === 0 && (1/x) === -Infinity);
  
  var convertI32PairToI53 = (lo, hi) => {
      // This function should not be getting called with too large unsigned numbers
      // in high part (if hi >= 0x7FFFFFFFF, one should have been calling
      // convertU32PairToI53())
      assert(hi === (hi|0));
      return (lo >>> 0) + hi * 4294967296;
    };
  
  var convertU32PairToI53 = (lo, hi) => {
      return (lo >>> 0) + (hi >>> 0) * 4294967296;
    };
  
  var reSign = (value, bits) => {
      if (value <= 0) {
        return value;
      }
      var half = bits <= 32 ? Math.abs(1 << (bits-1)) // abs is needed if bits == 32
                            : Math.pow(2, bits-1);
      // for huge values, we can hit the precision limit and always get true here.
      // so don't do that but, in general there is no perfect solution here. With
      // 64-bit ints, we get rounding and errors
      // TODO: In i64 mode 1, resign the two parts separately and safely
      if (value >= half && (bits <= 32 || value > half)) {
        // Cannot bitshift half, as it may be at the limit of the bits JS uses in
        // bitshifts
        value = -2*half + value;
      }
      return value;
    };
  
  var unSign = (value, bits) => {
      if (value >= 0) {
        return value;
      }
      // Need some trickery, since if bits == 32, we are right at the limit of the
      // bits JS uses in bitshifts
      return bits <= 32 ? 2*Math.abs(1 << (bits-1)) + value
                        : Math.pow(2, bits)         + value;
    };
  
  var strLen = (ptr) => {
      var end = ptr;
      while (HEAPU8[end]) ++end;
      return end - ptr;
    };
  
  var formatString = (format, varargs) => {
      assert((varargs & 3) === 0);
      var textIndex = format;
      var argIndex = varargs;
      // This must be called before reading a double or i64 vararg. It will bump the pointer properly.
      // It also does an assert on i32 values, so it's nice to call it before all varargs calls.
      function prepVararg(ptr, type) {
        if (type === 'double' || type === 'i64') {
          // move so the load is aligned
          if (ptr & 7) {
            assert((ptr & 7) === 4);
            ptr += 4;
          }
        } else {
          assert((ptr & 3) === 0);
        }
        return ptr;
      }
      function getNextArg(type) {
        // NOTE: Explicitly ignoring type safety. Otherwise this fails:
        //       int x = 4; printf("%c\n", (char)x);
        var ret;
        argIndex = prepVararg(argIndex, type);
        if (type === 'double') {
          ret = HEAPF64[((argIndex)>>3)];
          argIndex += 8;
        } else if (type == 'i64') {
          ret = [HEAP32[((argIndex)>>2)],
                 HEAP32[(((argIndex)+(4))>>2)]];
          argIndex += 8;
        } else {
          assert((argIndex & 3) === 0);
          type = 'i32'; // varargs are always i32, i64, or double
          ret = HEAP32[((argIndex)>>2)];
          argIndex += 4;
        }
        return ret;
      }
  
      var ret = [];
      var curr, next, currArg;
      while (1) {
        var startTextIndex = textIndex;
        curr = HEAP8[textIndex];
        if (curr === 0) break;
        next = HEAP8[textIndex+1];
        if (curr == 37) {
          // Handle flags.
          var flagAlwaysSigned = false;
          var flagLeftAlign = false;
          var flagAlternative = false;
          var flagZeroPad = false;
          var flagPadSign = false;
          flagsLoop: while (1) {
            switch (next) {
              case 43:
                flagAlwaysSigned = true;
                break;
              case 45:
                flagLeftAlign = true;
                break;
              case 35:
                flagAlternative = true;
                break;
              case 48:
                if (flagZeroPad) {
                  break flagsLoop;
                } else {
                  flagZeroPad = true;
                  break;
                }
              case 32:
                flagPadSign = true;
                break;
              default:
                break flagsLoop;
            }
            textIndex++;
            next = HEAP8[textIndex+1];
          }
  
          // Handle width.
          var width = 0;
          if (next == 42) {
            width = getNextArg('i32');
            textIndex++;
            next = HEAP8[textIndex+1];
          } else {
            while (next >= 48 && next <= 57) {
              width = width * 10 + (next - 48);
              textIndex++;
              next = HEAP8[textIndex+1];
            }
          }
  
          // Handle precision.
          var precisionSet = false, precision = -1;
          if (next == 46) {
            precision = 0;
            precisionSet = true;
            textIndex++;
            next = HEAP8[textIndex+1];
            if (next == 42) {
              precision = getNextArg('i32');
              textIndex++;
            } else {
              while (1) {
                var precisionChr = HEAP8[textIndex+1];
                if (precisionChr < 48 ||
                    precisionChr > 57) break;
                precision = precision * 10 + (precisionChr - 48);
                textIndex++;
              }
            }
            next = HEAP8[textIndex+1];
          }
          if (precision < 0) {
            precision = 6; // Standard default.
            precisionSet = false;
          }
  
          // Handle integer sizes. WARNING: These assume a 32-bit architecture!
          var argSize;
          switch (String.fromCharCode(next)) {
            case 'h':
              var nextNext = HEAP8[textIndex+2];
              if (nextNext == 104) {
                textIndex++;
                argSize = 1; // char (actually i32 in varargs)
              } else {
                argSize = 2; // short (actually i32 in varargs)
              }
              break;
            case 'l':
              var nextNext = HEAP8[textIndex+2];
              if (nextNext == 108) {
                textIndex++;
                argSize = 8; // long long
              } else {
                argSize = 4; // long
              }
              break;
            case 'L': // long long
            case 'q': // int64_t
            case 'j': // intmax_t
              argSize = 8;
              break;
            case 'z': // size_t
            case 't': // ptrdiff_t
            case 'I': // signed ptrdiff_t or unsigned size_t
              argSize = 4;
              break;
            default:
              argSize = null;
          }
          if (argSize) textIndex++;
          next = HEAP8[textIndex+1];
  
          // Handle type specifier.
          switch (String.fromCharCode(next)) {
            case 'd': case 'i': case 'u': case 'o': case 'x': case 'X': case 'p': {
              // Integer.
              var signed = next == 100 || next == 105;
              argSize = argSize || 4;
              currArg = getNextArg('i' + (argSize * 8));
              var argText;
              // Flatten i64-1 [low, high] into a (slightly rounded) double
              if (argSize == 8) {
                currArg = next == 117 ? convertU32PairToI53(currArg[0], currArg[1]) : convertI32PairToI53(currArg[0], currArg[1]);
              }
              // Truncate to requested size.
              if (argSize <= 4) {
                var limit = Math.pow(256, argSize) - 1;
                currArg = (signed ? reSign : unSign)(currArg & limit, argSize * 8);
              }
              // Format the number.
              var currAbsArg = Math.abs(currArg);
              var prefix = '';
              if (next == 100 || next == 105) {
                argText = reSign(currArg, 8 * argSize).toString(10);
              } else if (next == 117) {
                argText = unSign(currArg, 8 * argSize).toString(10);
                currArg = Math.abs(currArg);
              } else if (next == 111) {
                argText = (flagAlternative ? '0' : '') + currAbsArg.toString(8);
              } else if (next == 120 || next == 88) {
                prefix = (flagAlternative && currArg != 0) ? '0x' : '';
                if (currArg < 0) {
                  // Represent negative numbers in hex as 2's complement.
                  currArg = -currArg;
                  argText = (currAbsArg - 1).toString(16);
                  var buffer = [];
                  for (var i = 0; i < argText.length; i++) {
                    buffer.push((0xF - parseInt(argText[i], 16)).toString(16));
                  }
                  argText = buffer.join('');
                  while (argText.length < argSize * 2) argText = 'f' + argText;
                } else {
                  argText = currAbsArg.toString(16);
                }
                if (next == 88) {
                  prefix = prefix.toUpperCase();
                  argText = argText.toUpperCase();
                }
              } else if (next == 112) {
                if (currAbsArg === 0) {
                  argText = '(nil)';
                } else {
                  prefix = '0x';
                  argText = currAbsArg.toString(16);
                }
              }
              if (precisionSet) {
                while (argText.length < precision) {
                  argText = '0' + argText;
                }
              }
  
              // Add sign if needed
              if (currArg >= 0) {
                if (flagAlwaysSigned) {
                  prefix = '+' + prefix;
                } else if (flagPadSign) {
                  prefix = ' ' + prefix;
                }
              }
  
              // Move sign to prefix so we zero-pad after the sign
              if (argText.charAt(0) == '-') {
                prefix = '-' + prefix;
                argText = argText.slice(1);
              }
  
              // Add padding.
              while (prefix.length + argText.length < width) {
                if (flagLeftAlign) {
                  argText += ' ';
                } else {
                  if (flagZeroPad) {
                    argText = '0' + argText;
                  } else {
                    prefix = ' ' + prefix;
                  }
                }
              }
  
              // Insert the result into the buffer.
              argText = prefix + argText;
              argText.split('').forEach((chr) => ret.push(chr.charCodeAt(0)));
              break;
            }
            case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': {
              // Float.
              currArg = getNextArg('double');
              var argText;
              if (isNaN(currArg)) {
                argText = 'nan';
                flagZeroPad = false;
              } else if (!isFinite(currArg)) {
                argText = (currArg < 0 ? '-' : '') + 'inf';
                flagZeroPad = false;
              } else {
                var isGeneral = false;
                var effectivePrecision = Math.min(precision, 20);
  
                // Convert g/G to f/F or e/E, as per:
                // http://pubs.opengroup.org/onlinepubs/9699919799/functions/printf.html
                if (next == 103 || next == 71) {
                  isGeneral = true;
                  precision = precision || 1;
                  var exponent = parseInt(currArg.toExponential(effectivePrecision).split('e')[1], 10);
                  if (precision > exponent && exponent >= -4) {
                    next = ((next == 103) ? 'f' : 'F').charCodeAt(0);
                    precision -= exponent + 1;
                  } else {
                    next = ((next == 103) ? 'e' : 'E').charCodeAt(0);
                    precision--;
                  }
                  effectivePrecision = Math.min(precision, 20);
                }
  
                if (next == 101 || next == 69) {
                  argText = currArg.toExponential(effectivePrecision);
                  // Make sure the exponent has at least 2 digits.
                  if (/[eE][-+]\d$/.test(argText)) {
                    argText = argText.slice(0, -1) + '0' + argText.slice(-1);
                  }
                } else if (next == 102 || next == 70) {
                  argText = currArg.toFixed(effectivePrecision);
                  if (currArg === 0 && reallyNegative(currArg)) {
                    argText = '-' + argText;
                  }
                }
  
                var parts = argText.split('e');
                if (isGeneral && !flagAlternative) {
                  // Discard trailing zeros and periods.
                  while (parts[0].length > 1 && parts[0].includes('.') &&
                         (parts[0].slice(-1) == '0' || parts[0].slice(-1) == '.')) {
                    parts[0] = parts[0].slice(0, -1);
                  }
                } else {
                  // Make sure we have a period in alternative mode.
                  if (flagAlternative && argText.indexOf('.') == -1) parts[0] += '.';
                  // Zero pad until required precision.
                  while (precision > effectivePrecision++) parts[0] += '0';
                }
                argText = parts[0] + (parts.length > 1 ? 'e' + parts[1] : '');
  
                // Capitalize 'E' if needed.
                if (next == 69) argText = argText.toUpperCase();
  
                // Add sign.
                if (currArg >= 0) {
                  if (flagAlwaysSigned) {
                    argText = '+' + argText;
                  } else if (flagPadSign) {
                    argText = ' ' + argText;
                  }
                }
              }
  
              // Add padding.
              while (argText.length < width) {
                if (flagLeftAlign) {
                  argText += ' ';
                } else {
                  if (flagZeroPad && (argText[0] == '-' || argText[0] == '+')) {
                    argText = argText[0] + '0' + argText.slice(1);
                  } else {
                    argText = (flagZeroPad ? '0' : ' ') + argText;
                  }
                }
              }
  
              // Adjust case.
              if (next < 97) argText = argText.toUpperCase();
  
              // Insert the result into the buffer.
              argText.split('').forEach((chr) => ret.push(chr.charCodeAt(0)));
              break;
            }
            case 's': {
              // String.
              var arg = getNextArg('i8*');
              var argLength = arg ? strLen(arg) : '(null)'.length;
              if (precisionSet) argLength = Math.min(argLength, precision);
              if (!flagLeftAlign) {
                while (argLength < width--) {
                  ret.push(32);
                }
              }
              if (arg) {
                for (var i = 0; i < argLength; i++) {
                  ret.push(HEAPU8[arg++]);
                }
              } else {
                ret = ret.concat(intArrayFromString('(null)'.slice(0, argLength), true));
              }
              if (flagLeftAlign) {
                while (argLength < width--) {
                  ret.push(32);
                }
              }
              break;
            }
            case 'c': {
              // Character.
              if (flagLeftAlign) ret.push(getNextArg('i8'));
              while (--width > 0) {
                ret.push(32);
              }
              if (!flagLeftAlign) ret.push(getNextArg('i8'));
              break;
            }
            case 'n': {
              // Write the length written so far to the next parameter.
              var ptr = getNextArg('i32*');
              HEAP32[((ptr)>>2)] = ret.length;
              break;
            }
            case '%': {
              // Literal percent sign.
              ret.push(curr);
              break;
            }
            default: {
              // Unknown specifiers remain untouched.
              for (var i = startTextIndex; i < textIndex + 2; i++) {
                ret.push(HEAP8[i]);
              }
            }
          }
          textIndex += 2;
          // TODO: Support a/A (hex float) and m (last error) specifiers.
          // TODO: Support %1${specifier} for arg selection.
        } else {
          ret.push(curr);
          textIndex += 1;
        }
      }
      return ret;
    };
  
  var jsStackTrace = () => new Error().stack.toString();
  
  /** @param {number=} flags */
  var getCallstack = (flags) => {
      var callstack = jsStackTrace();
  
      if (flags & 8) {
        warnOnce('emscripten_log with EM_LOG_C_STACK no longer has any effect');
      }
  
      // Process all lines:
      var lines = callstack.split('\n');
      callstack = '';
      // Extract components of form:
      // '       Object._main@http://server.com:4324:12'
      var firefoxRe = new RegExp('\\s*(.*?)@(.*?):([0-9]+):([0-9]+)');
      // Extract components of form:
      // '    at Object._main (http://server.com/file.html:4324:12)'
      var chromeRe = new RegExp('\\s*at (.*?) \\\((.*):(.*):(.*)\\\)');
  
      for (var line of lines) {
        var symbolName = '';
        var file = '';
        var lineno = 0;
        var column = 0;
  
        var parts = chromeRe.exec(line);
        if (parts?.length == 5) {
          symbolName = parts[1];
          file = parts[2];
          lineno = parts[3];
          column = parts[4];
        } else {
          parts = firefoxRe.exec(line);
          if (parts?.length >= 4) {
            symbolName = parts[1];
            file = parts[2];
            lineno = parts[3];
            // Old Firefox doesn't carry column information, but in new FF30, it
            // is present. See https://bugzilla.mozilla.org/show_bug.cgi?id=762556
            column = parts[4]|0;
          } else {
            // Was not able to extract this line for demangling/sourcemapping
            // purposes. Output it as-is.
            callstack += line + '\n';
            continue;
          }
        }
  
        // Find the symbols in the callstack that corresponds to the functions that
        // report callstack information, and remove everything up to these from the
        // output.
        if (symbolName == '_emscripten_log' || symbolName == '_emscripten_get_callstack') {
          callstack = '';
          continue;
        }
  
        if ((flags & 24)) {
          if (flags & 64) {
            file = file.substring(file.replace(/\\/g, "/").lastIndexOf('/')+1);
          }
          callstack += `    at ${symbolName} (${file}:${lineno}:${column})\n`;
        }
      }
      // Trim extra whitespace at the end of the output.
      callstack = callstack.replace(/\s+$/, '');
      return callstack;
    };
  var emscriptenLog = (flags, str) => {
      if (flags & 24) {
        str = str.replace(/\s+$/, ''); // Ensure the message and the callstack are joined cleanly with exactly one newline.
        str += (str.length > 0 ? '\n' : '') + getCallstack(flags);
      }
  
      if (flags & 1) {
        if (flags & 4) {
          console.error(str);
        } else if (flags & 2) {
          console.warn(str);
        } else if (flags & 512) {
          console.info(str);
        } else if (flags & 256) {
          console.debug(str);
        } else {
          console.log(str);
        }
      } else if (flags & 6) {
        err(str);
      } else {
        out(str);
      }
    };
  var _emscripten_log = (flags, format, varargs) => {
      var result = formatString(format, varargs);
      var str = UTF8ArrayToString(result);
      emscriptenLog(flags, str);
    };

  
  
  var doRequestFullscreen = (target, strategy) => {
      if (!JSEvents.fullscreenEnabled()) return -1;
      target = findEventTarget(target);
      if (!target) return -4;
  
      if (!target.requestFullscreen
        && !target.webkitRequestFullscreen
        ) {
        return -3;
      }
  
      // Queue this function call if we're not currently in an event handler and
      // the user saw it appropriate to do so.
      if (!JSEvents.canPerformEventHandlerRequests()) {
        if (strategy.deferUntilInEventHandler) {
          JSEvents.deferCall(JSEvents_requestFullscreen, 1 /* priority over pointer lock */, [target, strategy]);
          return 1;
        }
        return -2;
      }
  
      return JSEvents_requestFullscreen(target, strategy);
    };
  var _emscripten_request_fullscreen_strategy = (target, deferUntilInEventHandler, fullscreenStrategy) => {
      var strategy = {
        scaleMode: HEAP32[((fullscreenStrategy)>>2)],
        canvasResolutionScaleMode: HEAP32[(((fullscreenStrategy)+(4))>>2)],
        filteringMode: HEAP32[(((fullscreenStrategy)+(8))>>2)],
        deferUntilInEventHandler,
        canvasResizedCallback: HEAP32[(((fullscreenStrategy)+(12))>>2)],
        canvasResizedCallbackUserData: HEAP32[(((fullscreenStrategy)+(16))>>2)]
      };
  
      return doRequestFullscreen(target, strategy);
    };

  
  
  var _emscripten_request_pointerlock = (target, deferUntilInEventHandler) => {
      target = findEventTarget(target);
      if (!target) return -4;
      if (!target.requestPointerLock
        ) {
        return -1;
      }
  
      // Queue this function call if we're not currently in an event handler and
      // the user saw it appropriate to do so.
      if (!JSEvents.canPerformEventHandlerRequests()) {
        if (deferUntilInEventHandler) {
          JSEvents.deferCall(requestPointerLock, 2 /* priority below fullscreen */, [target]);
          return 1;
        }
        return -2;
      }
  
      return requestPointerLock(target);
    };

  
  
  var growMemory = (size) => {
      var b = wasmMemory.buffer;
      var pages = ((size - b.byteLength + 65535) / 65536) | 0;
      try {
        // round size grow request up to wasm page size (fixed 64KB per spec)
        wasmMemory.grow(pages); // .grow() takes a delta compared to the previous size
        updateMemoryViews();
        return 1 /*success*/;
      } catch(e) {
        err(`growMemory: Attempted to grow heap from ${b.byteLength} bytes to ${size} bytes, but got error: ${e}`);
      }
      // implicit 0 return to save code size (caller will cast "undefined" into 0
      // anyhow)
    };
  var _emscripten_resize_heap = (requestedSize) => {
      var oldSize = HEAPU8.length;
      // With CAN_ADDRESS_2GB or MEMORY64, pointers are already unsigned.
      requestedSize >>>= 0;
      // With multithreaded builds, races can happen (another thread might increase the size
      // in between), so return a failure, and let the caller retry.
      assert(requestedSize > oldSize);
  
      // Memory resize rules:
      // 1.  Always increase heap size to at least the requested size, rounded up
      //     to next page multiple.
      // 2a. If MEMORY_GROWTH_LINEAR_STEP == -1, excessively resize the heap
      //     geometrically: increase the heap size according to
      //     MEMORY_GROWTH_GEOMETRIC_STEP factor (default +20%), At most
      //     overreserve by MEMORY_GROWTH_GEOMETRIC_CAP bytes (default 96MB).
      // 2b. If MEMORY_GROWTH_LINEAR_STEP != -1, excessively resize the heap
      //     linearly: increase the heap size by at least
      //     MEMORY_GROWTH_LINEAR_STEP bytes.
      // 3.  Max size for the heap is capped at 2048MB-WASM_PAGE_SIZE, or by
      //     MAXIMUM_MEMORY, or by ASAN limit, depending on which is smallest
      // 4.  If we were unable to allocate as much memory, it may be due to
      //     over-eager decision to excessively reserve due to (3) above.
      //     Hence if an allocation fails, cut down on the amount of excess
      //     growth, in an attempt to succeed to perform a smaller allocation.
  
      // A limit is set for how much we can grow. We should not exceed that
      // (the wasm binary specifies it, so if we tried, we'd fail anyhow).
      var maxHeapSize = getHeapMax();
      if (requestedSize > maxHeapSize) {
        err(`Cannot enlarge memory, requested ${requestedSize} bytes, but the limit is ${maxHeapSize} bytes!`);
        return false;
      }
  
      // Loop through potential heap size increases. If we attempt a too eager
      // reservation that fails, cut down on the attempted size and reserve a
      // smaller bump instead. (max 3 times, chosen somewhat arbitrarily)
      for (var cutDown = 1; cutDown <= 4; cutDown *= 2) {
        var overGrownHeapSize = oldSize * (1 + 0.2 / cutDown); // ensure geometric growth
        // but limit overreserving (default to capping at +96MB overgrowth at most)
        overGrownHeapSize = Math.min(overGrownHeapSize, requestedSize + 100663296 );
  
        var newSize = Math.min(maxHeapSize, alignMemory(Math.max(requestedSize, overGrownHeapSize), 65536));
  
        var replacement = growMemory(newSize);
        if (replacement) {
  
          return true;
        }
      }
      err(`Failed to grow the heap from ${oldSize} bytes to ${newSize} bytes, not enough memory!`);
      return false;
    };

  /** @suppress {checkTypes} */
  var _emscripten_sample_gamepad_data = () => {
      try {
        if (navigator.getGamepads) return (JSEvents.lastGamepadState = navigator.getGamepads())
          ? 0 : -1;
      } catch(e) {
        err(`navigator.getGamepads() exists, but failed to execute with exception ${e}. Disabling Gamepad access.`);
        navigator.getGamepads = null; // Disable getGamepads() so that it won't be attempted to be used again.
      }
      return -1;
    };

  
  
  
  var registerBeforeUnloadEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) => {
      var beforeUnloadEventHandlerFunc = (e = event) => {
        // Note: This is always called on the main browser thread, since it needs synchronously return a value!
        var confirmationMessage = getWasmTableEntry(callbackfunc)(eventTypeId, 0, userData);
  
        if (confirmationMessage) {
          confirmationMessage = UTF8ToString(confirmationMessage);
        }
        if (confirmationMessage) {
          e.preventDefault();
          e.returnValue = confirmationMessage;
          return confirmationMessage;
        }
      };
  
      var eventHandler = {
        target: findEventTarget(target),
        eventTypeString,
        callbackfunc,
        handlerFunc: beforeUnloadEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  var _emscripten_set_beforeunload_callback_on_thread = (userData, callbackfunc, targetThread) => {
      if (typeof onbeforeunload == 'undefined') return -1;
      // beforeunload callback can only be registered on the main browser thread, because the page will go away immediately after returning from the handler,
      // and there is no time to start proxying it anywhere.
      if (targetThread !== 1) return -5;
      return registerBeforeUnloadEventCallback(2, userData, true, callbackfunc, 28, "beforeunload");
    };

  
  
  
  
  var registerFocusEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.focusEvent ||= _malloc(256);
  
      var focusEventHandlerFunc = (e = event) => {
        var nodeName = JSEvents.getNodeNameForTarget(e.target);
        var id = e.target.id ? e.target.id : '';
  
        var focusEvent = JSEvents.focusEvent;
        stringToUTF8(nodeName, focusEvent + 0, 128);
        stringToUTF8(id, focusEvent + 128, 128);
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, focusEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target: findEventTarget(target),
        eventTypeString,
        callbackfunc,
        handlerFunc: focusEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  var _emscripten_set_blur_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerFocusEventCallback(target, userData, useCapture, callbackfunc, 12, "blur", targetThread);


  var _emscripten_set_element_css_size = (target, width, height) => {
      target = findEventTarget(target);
      if (!target) return -4;
  
      target.style.width = width + "px";
      target.style.height = height + "px";
  
      return 0;
    };

  var _emscripten_set_focus_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerFocusEventCallback(target, userData, useCapture, callbackfunc, 13, "focus", targetThread);

  
  
  
  var fillFullscreenChangeEventData = (eventStruct) => {
      var fullscreenElement = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement;
      var isFullscreen = !!fullscreenElement;
      // Assigning a boolean to HEAP32 with expected type coercion.
      /** @suppress{checkTypes} */
      HEAP8[eventStruct] = isFullscreen;
      HEAP8[(eventStruct)+(1)] = JSEvents.fullscreenEnabled();
      // If transitioning to fullscreen, report info about the element that is now fullscreen.
      // If transitioning to windowed mode, report info about the element that just was fullscreen.
      var reportedElement = isFullscreen ? fullscreenElement : JSEvents.previousFullscreenElement;
      var nodeName = JSEvents.getNodeNameForTarget(reportedElement);
      var id = reportedElement?.id || '';
      stringToUTF8(nodeName, eventStruct + 2, 128);
      stringToUTF8(id, eventStruct + 130, 128);
      HEAP32[(((eventStruct)+(260))>>2)] = reportedElement ? reportedElement.clientWidth : 0;
      HEAP32[(((eventStruct)+(264))>>2)] = reportedElement ? reportedElement.clientHeight : 0;
      HEAP32[(((eventStruct)+(268))>>2)] = screen.width;
      HEAP32[(((eventStruct)+(272))>>2)] = screen.height;
      if (isFullscreen) {
        JSEvents.previousFullscreenElement = fullscreenElement;
      }
    };
  
  
  var registerFullscreenChangeEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.fullscreenChangeEvent ||= _malloc(276);
  
      var fullscreenChangeEventhandlerFunc = (e = event) => {
        var fullscreenChangeEvent = JSEvents.fullscreenChangeEvent;
  
        fillFullscreenChangeEventData(fullscreenChangeEvent);
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, fullscreenChangeEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target,
        eventTypeString,
        callbackfunc,
        handlerFunc: fullscreenChangeEventhandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  
  var _emscripten_set_fullscreenchange_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) => {
      if (!JSEvents.fullscreenEnabled()) return -1;
      target = findEventTarget(target);
      if (!target) return -4;
  
      // Unprefixed Fullscreen API shipped in Chromium 71 (https://bugs.chromium.org/p/chromium/issues/detail?id=383813)
      // As of Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitfullscreenchange. TODO: revisit this check once Safari ships unprefixed version.
      registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, 19, "webkitfullscreenchange", targetThread);
  
      return registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, 19, "fullscreenchange", targetThread);
    };

  
  
  
  
  var registerGamepadEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.gamepadEvent ||= _malloc(1240);
  
      var gamepadEventHandlerFunc = (e = event) => {
        var gamepadEvent = JSEvents.gamepadEvent;
        fillGamepadEventData(gamepadEvent, e["gamepad"]);
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, gamepadEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target: findEventTarget(target),
        allowsDeferredCalls: true,
        eventTypeString,
        callbackfunc,
        handlerFunc: gamepadEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  
  var _emscripten_set_gamepadconnected_callback_on_thread = (userData, useCapture, callbackfunc, targetThread) => {
      if (_emscripten_sample_gamepad_data()) return -1;
      return registerGamepadEventCallback(2, userData, useCapture, callbackfunc, 26, "gamepadconnected", targetThread);
    };

  
  var _emscripten_set_gamepaddisconnected_callback_on_thread = (userData, useCapture, callbackfunc, targetThread) => {
      if (_emscripten_sample_gamepad_data()) return -1;
      return registerGamepadEventCallback(2, userData, useCapture, callbackfunc, 27, "gamepaddisconnected", targetThread);
    };

  
  
  
  
  var registerKeyEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.keyEvent ||= _malloc(160);
  
      var keyEventHandlerFunc = (e) => {
        assert(e);
  
        var keyEventData = JSEvents.keyEvent;
        HEAPF64[((keyEventData)>>3)] = e.timeStamp;
  
        var idx = ((keyEventData)>>2);
  
        HEAP32[idx + 2] = e.location;
        HEAP8[keyEventData + 12] = e.ctrlKey;
        HEAP8[keyEventData + 13] = e.shiftKey;
        HEAP8[keyEventData + 14] = e.altKey;
        HEAP8[keyEventData + 15] = e.metaKey;
        HEAP8[keyEventData + 16] = e.repeat;
        HEAP32[idx + 5] = e.charCode;
        HEAP32[idx + 6] = e.keyCode;
        HEAP32[idx + 7] = e.which;
        stringToUTF8(e.key || '', keyEventData + 32, 32);
        stringToUTF8(e.code || '', keyEventData + 64, 32);
        stringToUTF8(e.char || '', keyEventData + 96, 32);
        stringToUTF8(e.locale || '', keyEventData + 128, 32);
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, keyEventData, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target: findEventTarget(target),
        eventTypeString,
        callbackfunc,
        handlerFunc: keyEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  var _emscripten_set_keydown_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerKeyEventCallback(target, userData, useCapture, callbackfunc, 2, "keydown", targetThread);

  var _emscripten_set_keypress_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerKeyEventCallback(target, userData, useCapture, callbackfunc, 1, "keypress", targetThread);

  var _emscripten_set_keyup_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerKeyEventCallback(target, userData, useCapture, callbackfunc, 3, "keyup", targetThread);

  
  var _emscripten_set_main_loop = (func, fps, simulateInfiniteLoop) => {
      var iterFunc = getWasmTableEntry(func);
      setMainLoop(iterFunc, fps, simulateInfiniteLoop);
    };

  
  var fillMouseEventData = (eventStruct, e, target) => {
      assert(eventStruct % 4 == 0);
      HEAPF64[((eventStruct)>>3)] = e.timeStamp;
      var idx = ((eventStruct)>>2);
      HEAP32[idx + 2] = e.screenX;
      HEAP32[idx + 3] = e.screenY;
      HEAP32[idx + 4] = e.clientX;
      HEAP32[idx + 5] = e.clientY;
      HEAP8[eventStruct + 24] = e.ctrlKey;
      HEAP8[eventStruct + 25] = e.shiftKey;
      HEAP8[eventStruct + 26] = e.altKey;
      HEAP8[eventStruct + 27] = e.metaKey;
      HEAP16[idx*2 + 14] = e.button;
      HEAP16[idx*2 + 15] = e.buttons;
  
      HEAP32[idx + 8] = e["movementX"]
        ;
  
      HEAP32[idx + 9] = e["movementY"]
        ;
  
      // Note: rect contains doubles (truncated to placate SAFE_HEAP, which is the same behaviour when writing to HEAP32 anyway)
      var rect = getBoundingClientRect(target);
      HEAP32[idx + 10] = e.clientX - (rect.left | 0);
      HEAP32[idx + 11] = e.clientY - (rect.top  | 0);
    };
  
  
  
  var registerMouseEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.mouseEvent ||= _malloc(64);
      target = findEventTarget(target);
  
      var mouseEventHandlerFunc = (e = event) => {
        // TODO: Make this access thread safe, or this could update live while app is reading it.
        fillMouseEventData(JSEvents.mouseEvent, e, target);
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, JSEvents.mouseEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target,
        allowsDeferredCalls: eventTypeString != 'mousemove' && eventTypeString != 'mouseenter' && eventTypeString != 'mouseleave', // Mouse move events do not allow fullscreen/pointer lock requests to be handled in them!
        eventTypeString,
        callbackfunc,
        handlerFunc: mouseEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  var _emscripten_set_mousedown_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerMouseEventCallback(target, userData, useCapture, callbackfunc, 5, "mousedown", targetThread);

  var _emscripten_set_mouseenter_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerMouseEventCallback(target, userData, useCapture, callbackfunc, 33, "mouseenter", targetThread);

  var _emscripten_set_mouseleave_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerMouseEventCallback(target, userData, useCapture, callbackfunc, 34, "mouseleave", targetThread);

  var _emscripten_set_mousemove_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerMouseEventCallback(target, userData, useCapture, callbackfunc, 8, "mousemove", targetThread);

  var _emscripten_set_mouseup_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerMouseEventCallback(target, userData, useCapture, callbackfunc, 6, "mouseup", targetThread);

  
  
  var fillPointerlockChangeEventData = (eventStruct) => {
      var pointerLockElement = document.pointerLockElement || document.mozPointerLockElement || document.webkitPointerLockElement || document.msPointerLockElement;
      var isPointerlocked = !!pointerLockElement;
      // Assigning a boolean to HEAP32 with expected type coercion.
      /** @suppress{checkTypes} */
      HEAP8[eventStruct] = isPointerlocked;
      var nodeName = JSEvents.getNodeNameForTarget(pointerLockElement);
      var id = pointerLockElement?.id || '';
      stringToUTF8(nodeName, eventStruct + 1, 128);
      stringToUTF8(id, eventStruct + 129, 128);
    };
  
  
  var registerPointerlockChangeEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.pointerlockChangeEvent ||= _malloc(257);
  
      var pointerlockChangeEventHandlerFunc = (e = event) => {
        var pointerlockChangeEvent = JSEvents.pointerlockChangeEvent;
        fillPointerlockChangeEventData(pointerlockChangeEvent);
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, pointerlockChangeEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target,
        eventTypeString,
        callbackfunc,
        handlerFunc: pointerlockChangeEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  
  /** @suppress {missingProperties} */
  var _emscripten_set_pointerlockchange_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) => {
      // TODO: Currently not supported in pthreads or in --proxy-to-worker mode. (In pthreads mode, document object is not defined)
      if (!document || !document.body || (!document.body.requestPointerLock && !document.body.mozRequestPointerLock && !document.body.webkitRequestPointerLock && !document.body.msRequestPointerLock)) {
        return -1;
      }
  
      target = findEventTarget(target);
      if (!target) return -4;
      registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, 20, "mozpointerlockchange", targetThread);
      registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, 20, "webkitpointerlockchange", targetThread);
      registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, 20, "mspointerlockchange", targetThread);
      return registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, 20, "pointerlockchange", targetThread);
    };

  
  
  
  var registerUiEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.uiEvent ||= _malloc(36);
  
      target = findEventTarget(target);
  
      var uiEventHandlerFunc = (e = event) => {
        if (e.target != target) {
          // Never take ui events such as scroll via a 'bubbled' route, but always from the direct element that
          // was targeted. Otherwise e.g. if app logs a message in response to a page scroll, the Emscripten log
          // message box could cause to scroll, generating a new (bubbled) scroll message, causing a new log print,
          // causing a new scroll, etc..
          return;
        }
        var b = document.body; // Take document.body to a variable, Closure compiler does not outline access to it on its own.
        if (!b) {
          // During a page unload 'body' can be null, with "Cannot read property 'clientWidth' of null" being thrown
          return;
        }
        var uiEvent = JSEvents.uiEvent;
        HEAP32[((uiEvent)>>2)] = 0; // always zero for resize and scroll
        HEAP32[(((uiEvent)+(4))>>2)] = b.clientWidth;
        HEAP32[(((uiEvent)+(8))>>2)] = b.clientHeight;
        HEAP32[(((uiEvent)+(12))>>2)] = innerWidth;
        HEAP32[(((uiEvent)+(16))>>2)] = innerHeight;
        HEAP32[(((uiEvent)+(20))>>2)] = outerWidth;
        HEAP32[(((uiEvent)+(24))>>2)] = outerHeight;
        HEAP32[(((uiEvent)+(28))>>2)] = pageXOffset | 0; // scroll offsets are float
        HEAP32[(((uiEvent)+(32))>>2)] = pageYOffset | 0;
        if (getWasmTableEntry(callbackfunc)(eventTypeId, uiEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target,
        eventTypeString,
        callbackfunc,
        handlerFunc: uiEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  var _emscripten_set_resize_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerUiEventCallback(target, userData, useCapture, callbackfunc, 10, "resize", targetThread);

  
  
  
  
  var registerTouchEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.touchEvent ||= _malloc(1552);
  
      target = findEventTarget(target);
  
      var touchEventHandlerFunc = (e) => {
        assert(e);
        var t, touches = {}, et = e.touches;
        // To ease marshalling different kinds of touches that browser reports (all touches are listed in e.touches,
        // only changed touches in e.changedTouches, and touches on target at a.targetTouches), mark a boolean in
        // each Touch object so that we can later loop only once over all touches we see to marshall over to Wasm.
  
        for (let t of et) {
          // Browser might recycle the generated Touch objects between each frame (Firefox on Android), so reset any
          // changed/target states we may have set from previous frame.
          t.isChanged = t.onTarget = 0;
          touches[t.identifier] = t;
        }
        // Mark which touches are part of the changedTouches list.
        for (let t of e.changedTouches) {
          t.isChanged = 1;
          touches[t.identifier] = t;
        }
        // Mark which touches are part of the targetTouches list.
        for (let t of e.targetTouches) {
          touches[t.identifier].onTarget = 1;
        }
  
        var touchEvent = JSEvents.touchEvent;
        HEAPF64[((touchEvent)>>3)] = e.timeStamp;
        HEAP8[touchEvent + 12] = e.ctrlKey;
        HEAP8[touchEvent + 13] = e.shiftKey;
        HEAP8[touchEvent + 14] = e.altKey;
        HEAP8[touchEvent + 15] = e.metaKey;
        var idx = touchEvent + 16;
        var targetRect = getBoundingClientRect(target);
        var numTouches = 0;
        for (let t of Object.values(touches)) {
          var idx32 = ((idx)>>2); // Pre-shift the ptr to index to HEAP32 to save code size
          HEAP32[idx32 + 0] = t.identifier;
          HEAP32[idx32 + 1] = t.screenX;
          HEAP32[idx32 + 2] = t.screenY;
          HEAP32[idx32 + 3] = t.clientX;
          HEAP32[idx32 + 4] = t.clientY;
          HEAP32[idx32 + 5] = t.pageX;
          HEAP32[idx32 + 6] = t.pageY;
          HEAP8[idx + 28] = t.isChanged;
          HEAP8[idx + 29] = t.onTarget;
          HEAP32[idx32 + 8] = t.clientX - (targetRect.left | 0);
          HEAP32[idx32 + 9] = t.clientY - (targetRect.top  | 0);
  
          idx += 48;
  
          if (++numTouches > 31) {
            break;
          }
        }
        HEAP32[(((touchEvent)+(8))>>2)] = numTouches;
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, touchEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target,
        allowsDeferredCalls: eventTypeString == 'touchstart' || eventTypeString == 'touchend',
        eventTypeString,
        callbackfunc,
        handlerFunc: touchEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  var _emscripten_set_touchcancel_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerTouchEventCallback(target, userData, useCapture, callbackfunc, 25, "touchcancel", targetThread);

  var _emscripten_set_touchend_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerTouchEventCallback(target, userData, useCapture, callbackfunc, 23, "touchend", targetThread);

  var _emscripten_set_touchmove_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerTouchEventCallback(target, userData, useCapture, callbackfunc, 24, "touchmove", targetThread);

  var _emscripten_set_touchstart_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerTouchEventCallback(target, userData, useCapture, callbackfunc, 22, "touchstart", targetThread);

  
  var fillVisibilityChangeEventData = (eventStruct) => {
      var visibilityStates = [ "hidden", "visible", "prerender", "unloaded" ];
      var visibilityState = visibilityStates.indexOf(document.visibilityState);
  
      // Assigning a boolean to HEAP32 with expected type coercion.
      /** @suppress{checkTypes} */
      HEAP8[eventStruct] = document.hidden;
      HEAP32[(((eventStruct)+(4))>>2)] = visibilityState;
    };
  
  
  var registerVisibilityChangeEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.visibilityChangeEvent ||= _malloc(8);
  
      var visibilityChangeEventHandlerFunc = (e = event) => {
        var visibilityChangeEvent = JSEvents.visibilityChangeEvent;
  
        fillVisibilityChangeEventData(visibilityChangeEvent);
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, visibilityChangeEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target,
        eventTypeString,
        callbackfunc,
        handlerFunc: visibilityChangeEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  
  var _emscripten_set_visibilitychange_callback_on_thread = (userData, useCapture, callbackfunc, targetThread) => {
    if (!specialHTMLTargets[1]) {
      return -4;
    }
      return registerVisibilityChangeEventCallback(specialHTMLTargets[1], userData, useCapture, callbackfunc, 21, "visibilitychange", targetThread);
    };

  
  
  
  var registerWheelEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.wheelEvent ||= _malloc(96);
  
      // The DOM Level 3 events spec event 'wheel'
      var wheelHandlerFunc = (e = event) => {
        var wheelEvent = JSEvents.wheelEvent;
        fillMouseEventData(wheelEvent, e, target);
        HEAPF64[(((wheelEvent)+(64))>>3)] = e["deltaX"];
        HEAPF64[(((wheelEvent)+(72))>>3)] = e["deltaY"];
        HEAPF64[(((wheelEvent)+(80))>>3)] = e["deltaZ"];
        HEAP32[(((wheelEvent)+(88))>>2)] = e["deltaMode"];
        if (getWasmTableEntry(callbackfunc)(eventTypeId, wheelEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target,
        allowsDeferredCalls: true,
        eventTypeString,
        callbackfunc,
        handlerFunc: wheelHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  
  var _emscripten_set_wheel_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) => {
      target = findEventTarget(target);
      if (!target) return -4;
      if (typeof target.onwheel != 'undefined') {
        return registerWheelEventCallback(target, userData, useCapture, callbackfunc, 9, "wheel", targetThread);
      } else {
        return -1;
      }
    };

  
  var _emscripten_set_window_title = (title) => document.title = UTF8ToString(title);

  var _emscripten_sleep = () => {
      throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_sleep';
    };

  
  
  class HandleAllocator {
      allocated = [undefined];
      freelist = [];
      get(id) {
        assert(this.allocated[id] !== undefined, `invalid handle: ${id}`);
        return this.allocated[id];
      }
      has(id) {
        return this.allocated[id] !== undefined;
      }
      allocate(handle) {
        var id = this.freelist.pop() || this.allocated.length;
        this.allocated[id] = handle;
        return id;
      }
      free(id) {
        assert(this.allocated[id] !== undefined);
        // Set the slot to `undefined` rather than using `delete` here since
        // apparently arrays with holes in them can be less efficient.
        this.allocated[id] = undefined;
        this.freelist.push(id);
      }
    }
  var Fetch = {
  openDatabase(dbname, dbversion, onsuccess, onerror) {
      try {
        var openRequest = indexedDB.open(dbname, dbversion);
      } catch (e) {
        return onerror(e);
      }
  
      openRequest.onupgradeneeded = (event) => {
        var db = /** @type {IDBDatabase} */ (event.target.result);
        if (db.objectStoreNames.contains('FILES')) {
          db.deleteObjectStore('FILES');
        }
        db.createObjectStore('FILES');
      };
      openRequest.onsuccess = (event) => onsuccess(event.target.result);
      openRequest.onerror = onerror;
    },
  init() {
      Fetch.xhrs = new HandleAllocator();
      var onsuccess = (db) => {
        Fetch.dbInstance = db;
        removeRunDependency('library_fetch_init');
      };
  
      var onerror = () => {
        Fetch.dbInstance = false;
        removeRunDependency('library_fetch_init');
      };
  
      addRunDependency('library_fetch_init');
      Fetch.openDatabase('emscripten_filesystem', 1, onsuccess, onerror);
    },
  };
  
  function fetchXHR(fetch, onsuccess, onerror, onprogress, onreadystatechange) {
    var url = HEAPU32[(((fetch)+(8))>>2)];
    if (!url) {
      onerror(fetch, 0, 'no url specified!');
      return;
    }
    var url_ = UTF8ToString(url);
  
    var fetch_attr = fetch + 108;
    var requestMethod = UTF8ToString(fetch_attr + 0);
    requestMethod ||= 'GET';
    var timeoutMsecs = HEAPU32[(((fetch_attr)+(56))>>2)];
    var userName = HEAPU32[(((fetch_attr)+(68))>>2)];
    var password = HEAPU32[(((fetch_attr)+(72))>>2)];
    var requestHeaders = HEAPU32[(((fetch_attr)+(76))>>2)];
    var overriddenMimeType = HEAPU32[(((fetch_attr)+(80))>>2)];
    var dataPtr = HEAPU32[(((fetch_attr)+(84))>>2)];
    var dataLength = HEAPU32[(((fetch_attr)+(88))>>2)];
  
    var fetchAttributes = HEAPU32[(((fetch_attr)+(52))>>2)];
    var fetchAttrLoadToMemory = !!(fetchAttributes & 1);
    var fetchAttrStreamData = !!(fetchAttributes & 2);
    var fetchAttrSynchronous = !!(fetchAttributes & 64);
  
    var userNameStr = userName ? UTF8ToString(userName) : undefined;
    var passwordStr = password ? UTF8ToString(password) : undefined;
  
    var xhr = new XMLHttpRequest();
    xhr.withCredentials = !!HEAPU8[(fetch_attr)+(60)];;
    xhr.open(requestMethod, url_, !fetchAttrSynchronous, userNameStr, passwordStr);
    if (!fetchAttrSynchronous) xhr.timeout = timeoutMsecs; // XHR timeout field is only accessible in async XHRs, and must be set after .open() but before .send().
    xhr.url_ = url_; // Save the url for debugging purposes (and for comparing to the responseURL that server side advertised)
    assert(!fetchAttrStreamData, 'streaming uses moz-chunked-arraybuffer which is no longer supported; TODO: rewrite using fetch()');
    xhr.responseType = 'arraybuffer';
  
    if (overriddenMimeType) {
      var overriddenMimeTypeStr = UTF8ToString(overriddenMimeType);
      xhr.overrideMimeType(overriddenMimeTypeStr);
    }
    if (requestHeaders) {
      for (;;) {
        var key = HEAPU32[((requestHeaders)>>2)];
        if (!key) break;
        var value = HEAPU32[(((requestHeaders)+(4))>>2)];
        if (!value) break;
        requestHeaders += 8;
        var keyStr = UTF8ToString(key);
        var valueStr = UTF8ToString(value);
        xhr.setRequestHeader(keyStr, valueStr);
      }
    }
  
    var id = Fetch.xhrs.allocate(xhr);
    HEAPU32[((fetch)>>2)] = id;
    var data = (dataPtr && dataLength) ? HEAPU8.slice(dataPtr, dataPtr + dataLength) : null;
    // TODO: Support specifying custom headers to the request.
  
    // Share the code to save the response, as we need to do so both on success
    // and on error (despite an error, there may be a response, like a 404 page).
    // This receives a condition, which determines whether to save the xhr's
    // response, or just 0.
    function saveResponseAndStatus() {
      var ptr = 0;
      var ptrLen = 0;
      if (xhr.response && fetchAttrLoadToMemory && HEAPU32[(((fetch)+(12))>>2)] === 0) {
        ptrLen = xhr.response.byteLength;
      }
      if (ptrLen > 0) {
        // The data pointer malloc()ed here has the same lifetime as the emscripten_fetch_t structure itself has, and is
        // freed when emscripten_fetch_close() is called.
        ptr = _malloc(ptrLen);
        HEAPU8.set(new Uint8Array(/** @type{Array<number>} */(xhr.response)), ptr);
      }
      HEAPU32[(((fetch)+(12))>>2)] = ptr
      writeI53ToI64(fetch + 16, ptrLen);
      writeI53ToI64(fetch + 24, 0);
      var len = xhr.response ? xhr.response.byteLength : 0;
      if (len) {
        // If the final XHR.onload handler receives the bytedata to compute total length, report that,
        // otherwise don't write anything out here, which will retain the latest byte size reported in
        // the most recent XHR.onprogress handler.
        writeI53ToI64(fetch + 32, len);
      }
      HEAP16[(((fetch)+(40))>>1)] = xhr.readyState
      HEAP16[(((fetch)+(42))>>1)] = xhr.status
      if (xhr.statusText) stringToUTF8(xhr.statusText, fetch + 44, 64);
    }
  
    xhr.onload = (e) => {
      // check if xhr was aborted by user and don't try to call back
      if (!Fetch.xhrs.has(id)) {
        return;
      }
      saveResponseAndStatus();
      if (xhr.status >= 200 && xhr.status < 300) {
        onsuccess?.(fetch, xhr, e);
      } else {
        onerror?.(fetch, xhr, e);
      }
    };
    xhr.onerror = (e) => {
      // check if xhr was aborted by user and don't try to call back
      if (!Fetch.xhrs.has(id)) {
        return;
      }
      saveResponseAndStatus();
      onerror?.(fetch, xhr, e);
    };
    xhr.ontimeout = (e) => {
      // check if xhr was aborted by user and don't try to call back
      if (!Fetch.xhrs.has(id)) {
        return;
      }
      onerror?.(fetch, xhr, e);
    };
    xhr.onprogress = (e) => {
      // check if xhr was aborted by user and don't try to call back
      if (!Fetch.xhrs.has(id)) {
        return;
      }
      var ptrLen = (fetchAttrLoadToMemory && fetchAttrStreamData && xhr.response) ? xhr.response.byteLength : 0;
      var ptr = 0;
      if (ptrLen > 0 && fetchAttrLoadToMemory && fetchAttrStreamData) {
        assert(onprogress, 'When doing a streaming fetch, you should have an onprogress handler registered to receive the chunks!');
        // Allocate byte data in Emscripten heap for the streamed memory block (freed immediately after onprogress call)
        ptr = _malloc(ptrLen);
        HEAPU8.set(new Uint8Array(/** @type{Array<number>} */(xhr.response)), ptr);
      }
      HEAPU32[(((fetch)+(12))>>2)] = ptr
      writeI53ToI64(fetch + 16, ptrLen);
      writeI53ToI64(fetch + 24, e.loaded - ptrLen);
      writeI53ToI64(fetch + 32, e.total);
      HEAP16[(((fetch)+(40))>>1)] = xhr.readyState
      // If loading files from a source that does not give HTTP status code, assume success if we get data bytes
      if (xhr.readyState >= 3 && xhr.status === 0 && e.loaded > 0) xhr.status = 200;
      HEAP16[(((fetch)+(42))>>1)] = xhr.status
      if (xhr.statusText) stringToUTF8(xhr.statusText, fetch + 44, 64);
      onprogress?.(fetch, xhr, e);
      if (ptr) {
        _free(ptr);
      }
    };
    xhr.onreadystatechange = (e) => {
      // check if xhr was aborted by user and don't try to call back
      if (!Fetch.xhrs.has(id)) {
        
        return;
      }
      HEAP16[(((fetch)+(40))>>1)] = xhr.readyState
      if (xhr.readyState >= 2) {
        HEAP16[(((fetch)+(42))>>1)] = xhr.status
      }
      onreadystatechange?.(fetch, xhr, e);
    };
    try {
      xhr.send(data);
    } catch(e) {
      onerror?.(fetch, xhr, e);
    }
  }
  
  
  
  
  function fetchCacheData(/** @type {IDBDatabase} */ db, fetch, data, onsuccess, onerror) {
    if (!db) {
      onerror(fetch, 0, 'IndexedDB not available!');
      return;
    }
  
    var fetch_attr = fetch + 108;
    var destinationPath = HEAPU32[(((fetch_attr)+(64))>>2)];
    destinationPath ||= HEAPU32[(((fetch)+(8))>>2)];
    var destinationPathStr = UTF8ToString(destinationPath);
  
    try {
      var transaction = db.transaction(['FILES'], 'readwrite');
      var packages = transaction.objectStore('FILES');
      var putRequest = packages.put(data, destinationPathStr);
      putRequest.onsuccess = (event) => {
        HEAP16[(((fetch)+(40))>>1)] = 4 // Mimic XHR readyState 4 === 'DONE: The operation is complete'
        HEAP16[(((fetch)+(42))>>1)] = 200 // Mimic XHR HTTP status code 200 "OK"
        stringToUTF8("OK", fetch + 44, 64);
        onsuccess(fetch, 0, destinationPathStr);
      };
      putRequest.onerror = (error) => {
        // Most likely we got an error if IndexedDB is unwilling to store any more data for this page.
        // TODO: Can we identify and break down different IndexedDB-provided errors and convert those
        // to more HTTP status codes for more information?
        HEAP16[(((fetch)+(40))>>1)] = 4 // Mimic XHR readyState 4 === 'DONE: The operation is complete'
        HEAP16[(((fetch)+(42))>>1)] = 413 // Mimic XHR HTTP status code 413 "Payload Too Large"
        stringToUTF8("Payload Too Large", fetch + 44, 64);
        onerror(fetch, 0, error);
      };
    } catch(e) {
      onerror(fetch, 0, e);
    }
  }
  
  function fetchLoadCachedData(db, fetch, onsuccess, onerror) {
    if (!db) {
      onerror(fetch, 0, 'IndexedDB not available!');
      return;
    }
  
    var fetch_attr = fetch + 108;
    var path = HEAPU32[(((fetch_attr)+(64))>>2)];
    path ||= HEAPU32[(((fetch)+(8))>>2)];
    var pathStr = UTF8ToString(path);
  
    try {
      var transaction = db.transaction(['FILES'], 'readonly');
      var packages = transaction.objectStore('FILES');
      var getRequest = packages.get(pathStr);
      getRequest.onsuccess = (event) => {
        if (event.target.result) {
          var value = event.target.result;
          var len = value.byteLength || value.length;
          // The data pointer malloc()ed here has the same lifetime as the emscripten_fetch_t structure itself has, and is
          // freed when emscripten_fetch_close() is called.
          var ptr = _malloc(len);
          HEAPU8.set(new Uint8Array(value), ptr);
          HEAPU32[(((fetch)+(12))>>2)] = ptr;
          writeI53ToI64(fetch + 16, len);
          writeI53ToI64(fetch + 24, 0);
          writeI53ToI64(fetch + 32, len);
          HEAP16[(((fetch)+(40))>>1)] = 4 // Mimic XHR readyState 4 === 'DONE: The operation is complete'
          HEAP16[(((fetch)+(42))>>1)] = 200 // Mimic XHR HTTP status code 200 "OK"
          stringToUTF8("OK", fetch + 44, 64);
          onsuccess(fetch, 0, value);
        } else {
          // Succeeded to load, but the load came back with the value of undefined, treat that as an error since we never store undefined in db.
          HEAP16[(((fetch)+(40))>>1)] = 4 // Mimic XHR readyState 4 === 'DONE: The operation is complete'
          HEAP16[(((fetch)+(42))>>1)] = 404 // Mimic XHR HTTP status code 404 "Not Found"
          stringToUTF8("Not Found", fetch + 44, 64);
          onerror(fetch, 0, 'no data');
        }
      };
      getRequest.onerror = (error) => {
        HEAP16[(((fetch)+(40))>>1)] = 4 // Mimic XHR readyState 4 === 'DONE: The operation is complete'
        HEAP16[(((fetch)+(42))>>1)] = 404 // Mimic XHR HTTP status code 404 "Not Found"
        stringToUTF8("Not Found", fetch + 44, 64);
        onerror(fetch, 0, error);
      };
    } catch(e) {
      onerror(fetch, 0, e);
    }
  }
  
  function fetchDeleteCachedData(db, fetch, onsuccess, onerror) {
    if (!db) {
      onerror(fetch, 0, 'IndexedDB not available!');
      return;
    }
  
    var fetch_attr = fetch + 108;
    var path = HEAPU32[(((fetch_attr)+(64))>>2)];
    path ||= HEAPU32[(((fetch)+(8))>>2)];
  
    var pathStr = UTF8ToString(path);
  
    try {
      var transaction = db.transaction(['FILES'], 'readwrite');
      var packages = transaction.objectStore('FILES');
      var request = packages.delete(pathStr);
      request.onsuccess = (event) => {
        var value = event.target.result;
        HEAPU32[(((fetch)+(12))>>2)] = 0;
        writeI53ToI64(fetch + 16, 0);
        writeI53ToI64(fetch + 24, 0);
        writeI53ToI64(fetch + 32, 0);
        // Mimic XHR readyState 4 === 'DONE: The operation is complete'
        HEAP16[(((fetch)+(40))>>1)] = 4;
        // Mimic XHR HTTP status code 200 "OK"
        HEAP16[(((fetch)+(42))>>1)] = 200;
        stringToUTF8("OK", fetch + 44, 64);
        onsuccess(fetch, 0, value);
      };
      request.onerror = (error) => {
        HEAP16[(((fetch)+(40))>>1)] = 4 // Mimic XHR readyState 4 === 'DONE: The operation is complete'
        HEAP16[(((fetch)+(42))>>1)] = 404 // Mimic XHR HTTP status code 404 "Not Found"
        stringToUTF8("Not Found", fetch + 44, 64);
        onerror(fetch, 0, error);
      };
    } catch(e) {
      onerror(fetch, 0, e);
    }
  }
  
  
  function _emscripten_start_fetch(fetch, successcb, errorcb, progresscb, readystatechangecb) {
    // Avoid shutting down the runtime since we want to wait for the async
    // response.
    
  
    var fetch_attr = fetch + 108;
    var onsuccess = HEAPU32[(((fetch_attr)+(36))>>2)];
    var onerror = HEAPU32[(((fetch_attr)+(40))>>2)];
    var onprogress = HEAPU32[(((fetch_attr)+(44))>>2)];
    var onreadystatechange = HEAPU32[(((fetch_attr)+(48))>>2)];
    var fetchAttributes = HEAPU32[(((fetch_attr)+(52))>>2)];
    var fetchAttrSynchronous = !!(fetchAttributes & 64);
  
    function doCallback(f) {
      if (fetchAttrSynchronous) {
        f();
      } else {
        callUserCallback(f);
      }
    }
  
    var reportSuccess = (fetch, xhr, e) => {
      
      doCallback(() => {
        if (onsuccess) getWasmTableEntry(onsuccess)(fetch);
        else successcb?.(fetch);
      });
    };
  
    var reportProgress = (fetch, xhr, e) => {
      doCallback(() => {
        if (onprogress) getWasmTableEntry(onprogress)(fetch);
        else progresscb?.(fetch);
      });
    };
  
    var reportError = (fetch, xhr, e) => {
      
      doCallback(() => {
        if (onerror) getWasmTableEntry(onerror)(fetch);
        else errorcb?.(fetch);
      });
    };
  
    var reportReadyStateChange = (fetch, xhr, e) => {
      doCallback(() => {
        if (onreadystatechange) getWasmTableEntry(onreadystatechange)(fetch);
        else readystatechangecb?.(fetch);
      });
    };
  
    var performUncachedXhr = (fetch, xhr, e) => {
      fetchXHR(fetch, reportSuccess, reportError, reportProgress, reportReadyStateChange);
    };
  
    var cacheResultAndReportSuccess = (fetch, xhr, e) => {
      var storeSuccess = (fetch, xhr, e) => {
        
        doCallback(() => {
          if (onsuccess) getWasmTableEntry(onsuccess)(fetch);
          else successcb?.(fetch);
        });
      };
      var storeError = (fetch, xhr, e) => {
        
        doCallback(() => {
          if (onsuccess) getWasmTableEntry(onsuccess)(fetch);
          else successcb?.(fetch);
        });
      };
      fetchCacheData(Fetch.dbInstance, fetch, xhr.response, storeSuccess, storeError);
    };
  
    var performCachedXhr = (fetch, xhr, e) => {
      fetchXHR(fetch, cacheResultAndReportSuccess, reportError, reportProgress, reportReadyStateChange);
    };
  
    var requestMethod = UTF8ToString(fetch_attr + 0);
    var fetchAttrReplace = !!(fetchAttributes & 16);
    var fetchAttrPersistFile = !!(fetchAttributes & 4);
    var fetchAttrNoDownload = !!(fetchAttributes & 32);
    if (requestMethod === 'EM_IDB_STORE') {
      // TODO(?): Here we perform a clone of the data, because storing shared typed arrays to IndexedDB does not seem to be allowed.
      var ptr = HEAPU32[(((fetch_attr)+(84))>>2)];
      var size = HEAPU32[(((fetch_attr)+(88))>>2)];
      fetchCacheData(Fetch.dbInstance, fetch, HEAPU8.slice(ptr, ptr + size), reportSuccess, reportError);
    } else if (requestMethod === 'EM_IDB_DELETE') {
      fetchDeleteCachedData(Fetch.dbInstance, fetch, reportSuccess, reportError);
    } else if (!fetchAttrReplace) {
      fetchLoadCachedData(Fetch.dbInstance, fetch, reportSuccess, fetchAttrNoDownload ? reportError : (fetchAttrPersistFile ? performCachedXhr : performUncachedXhr));
    } else if (!fetchAttrNoDownload) {
      fetchXHR(fetch, fetchAttrPersistFile ? cacheResultAndReportSuccess : reportSuccess, reportError, reportProgress, reportReadyStateChange);
    } else {
      return 0; // todo: free
    }
    return fetch;
  }

  
  var webglPowerPreferences = ["default","low-power","high-performance"];
  
  
  /** @suppress {duplicate } */
  var _emscripten_webgl_do_create_context = (target, attributes) => {
      assert(attributes);
      var attr32 = ((attributes)>>2);
      var powerPreference = HEAP32[attr32 + (8>>2)];
      var contextAttributes = {
        'alpha': !!HEAP8[attributes + 0],
        'depth': !!HEAP8[attributes + 1],
        'stencil': !!HEAP8[attributes + 2],
        'antialias': !!HEAP8[attributes + 3],
        'premultipliedAlpha': !!HEAP8[attributes + 4],
        'preserveDrawingBuffer': !!HEAP8[attributes + 5],
        'powerPreference': webglPowerPreferences[powerPreference],
        'failIfMajorPerformanceCaveat': !!HEAP8[attributes + 12],
        // The following are not predefined WebGL context attributes in the WebGL specification, so the property names can be minified by Closure.
        majorVersion: HEAP32[attr32 + (16>>2)],
        minorVersion: HEAP32[attr32 + (20>>2)],
        enableExtensionsByDefault: HEAP8[attributes + 24],
        explicitSwapControl: HEAP8[attributes + 25],
        proxyContextToMainThread: HEAP32[attr32 + (28>>2)],
        renderViaOffscreenBackBuffer: HEAP8[attributes + 32]
      };
  
      //  TODO: Make these into hard errors at some point in the future
      if (contextAttributes.majorVersion !== 1 && contextAttributes.majorVersion !== 2) {
        err(`Invalid WebGL version requested: ${contextAttributes.majorVersion}`);
      }
      if (contextAttributes.majorVersion !== 2) {
        err('WebGL 1 requested but only WebGL 2 is supported (MIN_WEBGL_VERSION is 2)');
      }
  
      var canvas = findCanvasEventTarget(target);
  
      if (!canvas) {
        return 0;
      }
  
      if (contextAttributes.explicitSwapControl) {
        return 0;
      }
  
      var contextHandle = GL.createContext(canvas, contextAttributes);
      return contextHandle;
    };
  var _emscripten_webgl_create_context = _emscripten_webgl_do_create_context;

  var _emscripten_webgl_destroy_context = (contextHandle) => {
      if (GL.currentContext == contextHandle) GL.currentContext = 0;
      GL.deleteContext(contextHandle);
    };

  
  
  
  
  
  
  
  var _emscripten_webgl_enable_extension = (contextHandle, extension) => {
      var context = GL.getContext(contextHandle);
      var extString = UTF8ToString(extension);
      if (extString.startsWith('GL_')) extString = extString.slice(3); // Allow enabling extensions both with "GL_" prefix and without.
  
      // Switch-board that pulls in code for all GL extensions, even if those are not used :/
      // Build with -sGL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=0 to avoid this.
  
      if (extString == 'WEBGL_draw_instanced_base_vertex_base_instance') webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(GLctx);
      if (extString == 'WEBGL_multi_draw_instanced_base_vertex_base_instance') webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(GLctx);
  
      if (extString == 'WEBGL_multi_draw') webgl_enable_WEBGL_multi_draw(GLctx);
      if (extString == 'EXT_polygon_offset_clamp') webgl_enable_EXT_polygon_offset_clamp(GLctx);
      if (extString == 'EXT_clip_control') webgl_enable_EXT_clip_control(GLctx);
      if (extString == 'WEBGL_polygon_mode') webgl_enable_WEBGL_polygon_mode(GLctx);
  
      var ext = context.GLctx.getExtension(extString);
      return !!ext;
    };

  
  var _emscripten_webgl_get_context_attributes = (c, a) => {
      if (!a) return -5;
      c = GL.contexts[c];
      if (!c) return -3;
      var t = c.GLctx;
      if (!t) return -3;
      t = t.getContextAttributes();
  
      HEAP8[a] = t.alpha;
      HEAP8[(a)+(1)] = t.depth;
      HEAP8[(a)+(2)] = t.stencil;
      HEAP8[(a)+(3)] = t.antialias;
      HEAP8[(a)+(4)] = t.premultipliedAlpha;
      HEAP8[(a)+(5)] = t.preserveDrawingBuffer;
      var power = t['powerPreference'] && webglPowerPreferences.indexOf(t['powerPreference']);
      HEAP32[(((a)+(8))>>2)] = power;
      HEAP8[(a)+(12)] = t.failIfMajorPerformanceCaveat;
      HEAP32[(((a)+(16))>>2)] = c.version;
      HEAP32[(((a)+(20))>>2)] = 0;
      HEAP8[(a)+(24)] = c.attributes.enableExtensionsByDefault;
      return 0;
    };

  
  /** @suppress {duplicate } */
  var _emscripten_webgl_do_get_current_context = () => GL.currentContext ? GL.currentContext.handle : 0;
  var _emscripten_webgl_get_current_context = _emscripten_webgl_do_get_current_context;

  var _emscripten_webgl_make_context_current = (contextHandle) => {
      var success = GL.makeContextCurrent(contextHandle);
      return success ? 0 : -5;
    };

  var ENV = {
  };
  
  var getExecutableName = () => thisProgram || './this.program';
  var getEnvStrings = () => {
      if (!getEnvStrings.strings) {
        // Default values.
        // Browser language detection #8751
        var lang = ((typeof navigator == 'object' && navigator.languages && navigator.languages[0]) || 'C').replace('-', '_') + '.UTF-8';
        var env = {
          'USER': 'web_user',
          'LOGNAME': 'web_user',
          'PATH': '/',
          'PWD': '/',
          'HOME': '/home/web_user',
          'LANG': lang,
          '_': getExecutableName()
        };
        // Apply the user-provided values, if any.
        for (var x in ENV) {
          // x is a key in ENV; if ENV[x] is undefined, that means it was
          // explicitly set to be so. We allow user code to do that to
          // force variables with default values to remain unset.
          if (ENV[x] === undefined) delete env[x];
          else env[x] = ENV[x];
        }
        var strings = [];
        for (var x in env) {
          strings.push(`${x}=${env[x]}`);
        }
        getEnvStrings.strings = strings;
      }
      return getEnvStrings.strings;
    };
  
  var _environ_get = (__environ, environ_buf) => {
      var bufSize = 0;
      var envp = 0;
      for (var string of getEnvStrings()) {
        var ptr = environ_buf + bufSize;
        HEAPU32[(((__environ)+(envp))>>2)] = ptr;
        bufSize += stringToUTF8(string, ptr, Infinity) + 1;
        envp += 4;
      }
      return 0;
    };

  
  var _environ_sizes_get = (penviron_count, penviron_buf_size) => {
      var strings = getEnvStrings();
      HEAPU32[((penviron_count)>>2)] = strings.length;
      var bufSize = 0;
      for (var string of strings) {
        bufSize += lengthBytesUTF8(string) + 1;
      }
      HEAPU32[((penviron_buf_size)>>2)] = bufSize;
      return 0;
    };


  function _fd_close(fd) {
  try {
  
      var stream = SYSCALLS.getStreamFromFD(fd);
      FS.close(stream);
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return e.errno;
  }
  }

  /** @param {number=} offset */
  var doReadv = (stream, iov, iovcnt, offset) => {
      var ret = 0;
      for (var i = 0; i < iovcnt; i++) {
        var ptr = HEAPU32[((iov)>>2)];
        var len = HEAPU32[(((iov)+(4))>>2)];
        iov += 8;
        var curr = FS.read(stream, HEAP8, ptr, len, offset);
        if (curr < 0) return -1;
        ret += curr;
        if (curr < len) break; // nothing more to read
        if (typeof offset != 'undefined') {
          offset += curr;
        }
      }
      return ret;
    };
  
  function _fd_read(fd, iov, iovcnt, pnum) {
  try {
  
      var stream = SYSCALLS.getStreamFromFD(fd);
      var num = doReadv(stream, iov, iovcnt);
      HEAPU32[((pnum)>>2)] = num;
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return e.errno;
  }
  }

  
  function _fd_seek(fd, offset, whence, newOffset) {
    offset = bigintToI53Checked(offset);
  
  
  try {
  
      if (isNaN(offset)) return 61;
      var stream = SYSCALLS.getStreamFromFD(fd);
      FS.llseek(stream, offset, whence);
      HEAP64[((newOffset)>>3)] = BigInt(stream.position);
      if (stream.getdents && offset === 0 && whence === 0) stream.getdents = null; // reset readdir state
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return e.errno;
  }
  ;
  }

  /** @param {number=} offset */
  var doWritev = (stream, iov, iovcnt, offset) => {
      var ret = 0;
      for (var i = 0; i < iovcnt; i++) {
        var ptr = HEAPU32[((iov)>>2)];
        var len = HEAPU32[(((iov)+(4))>>2)];
        iov += 8;
        var curr = FS.write(stream, HEAP8, ptr, len, offset);
        if (curr < 0) return -1;
        ret += curr;
        if (curr < len) {
          // No more space to write.
          break;
        }
        if (typeof offset != 'undefined') {
          offset += curr;
        }
      }
      return ret;
    };
  
  function _fd_write(fd, iov, iovcnt, pnum) {
  try {
  
      var stream = SYSCALLS.getStreamFromFD(fd);
      var num = doWritev(stream, iov, iovcnt);
      HEAPU32[((pnum)>>2)] = num;
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return e.errno;
  }
  }



















































































  function _random_get(buffer, size) {
  try {
  
      randomFill(HEAPU8.subarray(buffer, buffer + size));
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return e.errno;
  }
  }





  var dynCall = (sig, ptr, args = [], promising = false) => {
      assert(!promising, 'async dynCall is not supported in this mode')
      assert(getWasmTableEntry(ptr), `missing table entry in dynCall: ${ptr}`);
      var func = getWasmTableEntry(ptr);
      var rtn = func(...args);
  
      function convert(rtn) {
        return rtn;
      }
  
      return convert(rtn);
    };





  var getCFunc = (ident) => {
      var func = Module['_' + ident]; // closure exported function
      assert(func, 'Cannot call unknown function ' + ident + ', make sure it is exported');
      return func;
    };
  
  var writeArrayToMemory = (array, buffer) => {
      assert(array.length >= 0, 'writeArrayToMemory array must have a length (should be an array or typed array)')
      HEAP8.set(array, buffer);
    };
  
  
  
  
  
  
    /**
     * @param {string|null=} returnType
     * @param {Array=} argTypes
     * @param {Arguments|Array=} args
     * @param {Object=} opts
     */
  var ccall = (ident, returnType, argTypes, args, opts) => {
      // For fast lookup of conversion functions
      var toC = {
        'string': (str) => {
          var ret = 0;
          if (str !== null && str !== undefined && str !== 0) { // null string
            ret = stringToUTF8OnStack(str);
          }
          return ret;
        },
        'array': (arr) => {
          var ret = stackAlloc(arr.length);
          writeArrayToMemory(arr, ret);
          return ret;
        }
      };
  
      function convertReturnValue(ret) {
        if (returnType === 'string') {
          return UTF8ToString(ret);
        }
        if (returnType === 'boolean') return Boolean(ret);
        return ret;
      }
  
      var func = getCFunc(ident);
      var cArgs = [];
      var stack = 0;
      assert(returnType !== 'array', 'Return type should not be "array".');
      if (args) {
        for (var i = 0; i < args.length; i++) {
          var converter = toC[argTypes[i]];
          if (converter) {
            if (stack === 0) stack = stackSave();
            cArgs[i] = converter(args[i]);
          } else {
            cArgs[i] = args[i];
          }
        }
      }
      var ret = func(...cArgs);
      function onDone(ret) {
        if (stack !== 0) stackRestore(stack);
        return convertReturnValue(ret);
      }
  
      ret = onDone(ret);
      return ret;
    };
  
    /**
     * @param {string=} returnType
     * @param {Array=} argTypes
     * @param {Object=} opts
     */
  var cwrap = (ident, returnType, argTypes, opts) => {
      return (...args) => ccall(ident, returnType, argTypes, args, opts);
    };



  var FS_createPath = (...args) => FS.createPath(...args);



  var FS_unlink = (...args) => FS.unlink(...args);

  var FS_createLazyFile = (...args) => FS.createLazyFile(...args);

  var FS_createDevice = (...args) => FS.createDevice(...args);

  var createContext = Browser.createContext;

  var incrementExceptionRefcount = (ptr) => ___cxa_increment_exception_refcount(ptr);

  var decrementExceptionRefcount = (ptr) => ___cxa_decrement_exception_refcount(ptr);

  
  
  
  
  
  var getExceptionMessageCommon = (ptr) => {
      var sp = stackSave();
      var type_addr_addr = stackAlloc(4);
      var message_addr_addr = stackAlloc(4);
      ___get_exception_message(ptr, type_addr_addr, message_addr_addr);
      var type_addr = HEAPU32[((type_addr_addr)>>2)];
      var message_addr = HEAPU32[((message_addr_addr)>>2)];
      var type = UTF8ToString(type_addr);
      _free(type_addr);
      var message;
      if (message_addr) {
        message = UTF8ToString(message_addr);
        _free(message_addr);
      }
      stackRestore(sp);
      return [type, message];
    };
  var getExceptionMessage = (ptr) => getExceptionMessageCommon(ptr);

  FS.createPreloadedFile = FS_createPreloadedFile;
  FS.staticInit();;

      Module['requestAnimationFrame'] = MainLoop.requestAnimationFrame;
      Module['pauseMainLoop'] = MainLoop.pause;
      Module['resumeMainLoop'] = MainLoop.resume;
      MainLoop.init();;
for (let i = 0; i < 32; ++i) tempFixedLengthArray.push(new Array(i));;
Fetch.init();;
// End JS library code

// include: postlibrary.js
// This file is included after the automatically-generated JS library code
// but before the wasm module is created.

{

  // Begin ATMODULES hooks
  if (Module['noExitRuntime']) noExitRuntime = Module['noExitRuntime'];
if (Module['preloadPlugins']) preloadPlugins = Module['preloadPlugins'];
if (Module['print']) out = Module['print'];
if (Module['printErr']) err = Module['printErr'];
if (Module['wasmBinary']) wasmBinary = Module['wasmBinary'];
  // End ATMODULES hooks

  checkIncomingModuleAPI();

  if (Module['arguments']) arguments_ = Module['arguments'];
  if (Module['thisProgram']) thisProgram = Module['thisProgram'];

  // Assertions on removed incoming Module JS APIs.
  assert(typeof Module['memoryInitializerPrefixURL'] == 'undefined', 'Module.memoryInitializerPrefixURL option was removed, use Module.locateFile instead');
  assert(typeof Module['pthreadMainPrefixURL'] == 'undefined', 'Module.pthreadMainPrefixURL option was removed, use Module.locateFile instead');
  assert(typeof Module['cdInitializerPrefixURL'] == 'undefined', 'Module.cdInitializerPrefixURL option was removed, use Module.locateFile instead');
  assert(typeof Module['filePackagePrefixURL'] == 'undefined', 'Module.filePackagePrefixURL option was removed, use Module.locateFile instead');
  assert(typeof Module['read'] == 'undefined', 'Module.read option was removed');
  assert(typeof Module['readAsync'] == 'undefined', 'Module.readAsync option was removed (modify readAsync in JS)');
  assert(typeof Module['readBinary'] == 'undefined', 'Module.readBinary option was removed (modify readBinary in JS)');
  assert(typeof Module['setWindowTitle'] == 'undefined', 'Module.setWindowTitle option was removed (modify emscripten_set_window_title in JS)');
  assert(typeof Module['TOTAL_MEMORY'] == 'undefined', 'Module.TOTAL_MEMORY has been renamed Module.INITIAL_MEMORY');
  assert(typeof Module['ENVIRONMENT'] == 'undefined', 'Module.ENVIRONMENT has been deprecated. To force the environment, use the ENVIRONMENT compile-time option (for example, -sENVIRONMENT=web or -sENVIRONMENT=node)');
  assert(typeof Module['STACK_SIZE'] == 'undefined', 'STACK_SIZE can no longer be set at runtime.  Use -sSTACK_SIZE at link time')
  // If memory is defined in wasm, the user can't provide it, or set INITIAL_MEMORY
  assert(typeof Module['wasmMemory'] == 'undefined', 'Use of `wasmMemory` detected.  Use -sIMPORTED_MEMORY to define wasmMemory externally');
  assert(typeof Module['INITIAL_MEMORY'] == 'undefined', 'Detected runtime INITIAL_MEMORY setting.  Use -sIMPORTED_MEMORY to define wasmMemory dynamically');

}

// Begin runtime exports
  Module['addRunDependency'] = addRunDependency;
  Module['removeRunDependency'] = removeRunDependency;
  Module['cwrap'] = cwrap;
  Module['setValue'] = setValue;
  Module['getValue'] = getValue;
  Module['createContext'] = createContext;
  Module['FS_createPreloadedFile'] = FS_createPreloadedFile;
  Module['FS_unlink'] = FS_unlink;
  Module['FS_createPath'] = FS_createPath;
  Module['FS_createDevice'] = FS_createDevice;
  Module['FS_createDataFile'] = FS_createDataFile;
  Module['FS_createLazyFile'] = FS_createLazyFile;
  var missingLibrarySymbols = [
  'writeI53ToI64Clamped',
  'writeI53ToI64Signaling',
  'writeI53ToU64Clamped',
  'writeI53ToU64Signaling',
  'convertI32PairToI53Checked',
  'getTempRet0',
  'inetPton4',
  'inetNtop4',
  'inetPton6',
  'inetNtop6',
  'readSockaddr',
  'writeSockaddr',
  'getDynCaller',
  'runtimeKeepalivePush',
  'runtimeKeepalivePop',
  'asmjsMangle',
  'getNativeTypeSize',
  'addOnInit',
  'addOnPostCtor',
  'addOnPreMain',
  'STACK_SIZE',
  'STACK_ALIGN',
  'POINTER_SIZE',
  'ASSERTIONS',
  'uleb128Encode',
  'sigToWasmTypes',
  'generateFuncType',
  'convertJsFunctionToWasm',
  'getEmptyTableSlot',
  'updateTableMap',
  'getFunctionAddress',
  'addFunction',
  'removeFunction',
  'intArrayToString',
  'AsciiToString',
  'stringToAscii',
  'UTF16ToString',
  'stringToUTF16',
  'lengthBytesUTF16',
  'UTF32ToString',
  'stringToUTF32',
  'lengthBytesUTF32',
  'fillDeviceOrientationEventData',
  'registerDeviceOrientationEventCallback',
  'fillDeviceMotionEventData',
  'registerDeviceMotionEventCallback',
  'screenOrientation',
  'fillOrientationChangeEventData',
  'registerOrientationChangeEventCallback',
  'hideEverythingExceptGivenElement',
  'restoreHiddenElements',
  'softFullscreenResizeWebGLRenderTarget',
  'registerPointerlockErrorEventCallback',
  'fillBatteryEventData',
  'battery',
  'registerBatteryEventCallback',
  'convertPCtoSourceLocation',
  'wasiRightsToMuslOFlags',
  'wasiOFlagsToMuslOFlags',
  'setImmediateWrapped',
  'safeRequestAnimationFrame',
  'clearImmediateWrapped',
  'registerPostMainLoop',
  'registerPreMainLoop',
  'getPromise',
  'makePromise',
  'idsToPromises',
  'makePromiseCallback',
  'Browser_asyncPrepareDataCounter',
  'arraySum',
  'addDays',
  'getSocketFromFD',
  'getSocketAddress',
  'FS_mkdirTree',
  '_setNetworkCallback',
  'writeGLArray',
  'registerWebGlEventCallback',
  'runAndAbortIfError',
  'ALLOC_NORMAL',
  'ALLOC_STACK',
  'allocate',
  'writeStringToMemory',
  'writeAsciiToMemory',
  'demangle',
  'stackTrace',
];
missingLibrarySymbols.forEach(missingLibrarySymbol)

  var unexportedSymbols = [
  'run',
  'out',
  'err',
  'callMain',
  'abort',
  'wasmMemory',
  'wasmExports',
  'HEAPF32',
  'HEAPF64',
  'HEAP8',
  'HEAPU8',
  'HEAP16',
  'HEAPU16',
  'HEAP32',
  'HEAPU32',
  'HEAP64',
  'HEAPU64',
  'writeStackCookie',
  'checkStackCookie',
  'writeI53ToI64',
  'readI53FromI64',
  'readI53FromU64',
  'convertI32PairToI53',
  'convertU32PairToI53',
  'INT53_MAX',
  'INT53_MIN',
  'bigintToI53Checked',
  'stackSave',
  'stackRestore',
  'stackAlloc',
  'setTempRet0',
  'ptrToString',
  'zeroMemory',
  'exitJS',
  'getHeapMax',
  'growMemory',
  'ENV',
  'ERRNO_CODES',
  'strError',
  'DNS',
  'Protocols',
  'Sockets',
  'timers',
  'warnOnce',
  'emscriptenLog',
  'readEmAsmArgsArray',
  'readEmAsmArgs',
  'runEmAsmFunction',
  'runMainThreadEmAsm',
  'jstoi_q',
  'getExecutableName',
  'listenOnce',
  'autoResumeAudioContext',
  'dynCall',
  'handleException',
  'keepRuntimeAlive',
  'callUserCallback',
  'maybeExit',
  'asyncLoad',
  'alignMemory',
  'mmapAlloc',
  'HandleAllocator',
  'wasmTable',
  'noExitRuntime',
  'addOnPreRun',
  'addOnExit',
  'addOnPostRun',
  'ccall',
  'freeTableIndexes',
  'functionsInTableMap',
  'reallyNegative',
  'unSign',
  'strLen',
  'reSign',
  'formatString',
  'PATH',
  'PATH_FS',
  'UTF8Decoder',
  'UTF8ArrayToString',
  'UTF8ToString',
  'stringToUTF8Array',
  'stringToUTF8',
  'lengthBytesUTF8',
  'intArrayFromString',
  'UTF16Decoder',
  'stringToNewUTF8',
  'stringToUTF8OnStack',
  'writeArrayToMemory',
  'JSEvents',
  'registerKeyEventCallback',
  'specialHTMLTargets',
  'maybeCStringToJsString',
  'findEventTarget',
  'findCanvasEventTarget',
  'getBoundingClientRect',
  'fillMouseEventData',
  'registerMouseEventCallback',
  'registerWheelEventCallback',
  'registerUiEventCallback',
  'registerFocusEventCallback',
  'fillFullscreenChangeEventData',
  'registerFullscreenChangeEventCallback',
  'JSEvents_requestFullscreen',
  'JSEvents_resizeCanvasForFullscreen',
  'registerRestoreOldStyle',
  'setLetterbox',
  'currentFullscreenStrategy',
  'restoreOldWindowedStyle',
  'doRequestFullscreen',
  'fillPointerlockChangeEventData',
  'registerPointerlockChangeEventCallback',
  'requestPointerLock',
  'fillVisibilityChangeEventData',
  'registerVisibilityChangeEventCallback',
  'registerTouchEventCallback',
  'fillGamepadEventData',
  'registerGamepadEventCallback',
  'registerBeforeUnloadEventCallback',
  'setCanvasElementSize',
  'getCanvasElementSize',
  'jsStackTrace',
  'getCallstack',
  'UNWIND_CACHE',
  'ExitStatus',
  'getEnvStrings',
  'checkWasiClock',
  'doReadv',
  'doWritev',
  'initRandomFill',
  'randomFill',
  'safeSetTimeout',
  'emSetImmediate',
  'emClearImmediate_deps',
  'emClearImmediate',
  'promiseMap',
  'uncaughtExceptionCount',
  'exceptionLast',
  'exceptionCaught',
  'ExceptionInfo',
  'findMatchingCatch',
  'getExceptionMessageCommon',
  'Browser',
  'requestFullscreen',
  'requestFullScreen',
  'setCanvasSize',
  'getUserMedia',
  'getPreloadedImageData__data',
  'wget',
  'MONTH_DAYS_REGULAR',
  'MONTH_DAYS_LEAP',
  'MONTH_DAYS_REGULAR_CUMULATIVE',
  'MONTH_DAYS_LEAP_CUMULATIVE',
  'isLeapYear',
  'ydayFromDate',
  'SYSCALLS',
  'preloadPlugins',
  'FS_modeStringToFlags',
  'FS_getMode',
  'FS_stdin_getChar_buffer',
  'FS_stdin_getChar',
  'FS_readFile',
  'FS',
  'FS_root',
  'FS_mounts',
  'FS_devices',
  'FS_streams',
  'FS_nextInode',
  'FS_nameTable',
  'FS_currentPath',
  'FS_initialized',
  'FS_ignorePermissions',
  'FS_filesystems',
  'FS_syncFSRequests',
  'FS_readFiles',
  'FS_lookupPath',
  'FS_getPath',
  'FS_hashName',
  'FS_hashAddNode',
  'FS_hashRemoveNode',
  'FS_lookupNode',
  'FS_createNode',
  'FS_destroyNode',
  'FS_isRoot',
  'FS_isMountpoint',
  'FS_isFile',
  'FS_isDir',
  'FS_isLink',
  'FS_isChrdev',
  'FS_isBlkdev',
  'FS_isFIFO',
  'FS_isSocket',
  'FS_flagsToPermissionString',
  'FS_nodePermissions',
  'FS_mayLookup',
  'FS_mayCreate',
  'FS_mayDelete',
  'FS_mayOpen',
  'FS_checkOpExists',
  'FS_nextfd',
  'FS_getStreamChecked',
  'FS_getStream',
  'FS_createStream',
  'FS_closeStream',
  'FS_dupStream',
  'FS_doSetAttr',
  'FS_chrdev_stream_ops',
  'FS_major',
  'FS_minor',
  'FS_makedev',
  'FS_registerDevice',
  'FS_getDevice',
  'FS_getMounts',
  'FS_syncfs',
  'FS_mount',
  'FS_unmount',
  'FS_lookup',
  'FS_mknod',
  'FS_statfs',
  'FS_statfsStream',
  'FS_statfsNode',
  'FS_create',
  'FS_mkdir',
  'FS_mkdev',
  'FS_symlink',
  'FS_rename',
  'FS_rmdir',
  'FS_readdir',
  'FS_readlink',
  'FS_stat',
  'FS_fstat',
  'FS_lstat',
  'FS_doChmod',
  'FS_chmod',
  'FS_lchmod',
  'FS_fchmod',
  'FS_doChown',
  'FS_chown',
  'FS_lchown',
  'FS_fchown',
  'FS_doTruncate',
  'FS_truncate',
  'FS_ftruncate',
  'FS_utime',
  'FS_open',
  'FS_close',
  'FS_isClosed',
  'FS_llseek',
  'FS_read',
  'FS_write',
  'FS_mmap',
  'FS_msync',
  'FS_ioctl',
  'FS_writeFile',
  'FS_cwd',
  'FS_chdir',
  'FS_createDefaultDirectories',
  'FS_createDefaultDevices',
  'FS_createSpecialDirectories',
  'FS_createStandardStreams',
  'FS_staticInit',
  'FS_init',
  'FS_quit',
  'FS_findObject',
  'FS_analyzePath',
  'FS_createFile',
  'FS_forceLoadFile',
  'FS_absolutePath',
  'FS_createFolder',
  'FS_createLink',
  'FS_joinPath',
  'FS_mmapAlloc',
  'FS_standardizePath',
  'MEMFS',
  'TTY',
  'PIPEFS',
  'SOCKFS',
  'tempFixedLengthArray',
  'miniTempWebGLFloatBuffers',
  'miniTempWebGLIntBuffers',
  'heapObjectForWebGLType',
  'toTypedArrayIndex',
  'webgl_enable_WEBGL_multi_draw',
  'webgl_enable_EXT_polygon_offset_clamp',
  'webgl_enable_EXT_clip_control',
  'webgl_enable_WEBGL_polygon_mode',
  'GL',
  'emscriptenWebGLGet',
  'computeUnpackAlignedImageSize',
  'colorChannelsInGlTextureFormat',
  'emscriptenWebGLGetTexPixelData',
  'emscriptenWebGLGetUniform',
  'webglGetUniformLocation',
  'webglPrepareUniformLocationsBeforeFirstUse',
  'webglGetLeftBracePos',
  'emscriptenWebGLGetVertexAttrib',
  '__glGetActiveAttribOrUniform',
  'AL',
  'GLUT',
  'EGL',
  'GLEW',
  'IDBStore',
  'emscriptenWebGLGetIndexed',
  'webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance',
  'webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance',
  'allocateUTF8',
  'allocateUTF8OnStack',
  'print',
  'printErr',
  'jstoi_s',
  'IDBFS',
  'Fetch',
  'fetchDeleteCachedData',
  'fetchLoadCachedData',
  'fetchCacheData',
  'fetchXHR',
];
unexportedSymbols.forEach(unexportedRuntimeSymbol);

  // End runtime exports
  // Begin JS library exports
  Module['incrementExceptionRefcount'] = incrementExceptionRefcount;
  Module['decrementExceptionRefcount'] = decrementExceptionRefcount;
  Module['getExceptionMessage'] = getExceptionMessage;
  // End JS library exports

// end include: postlibrary.js

function checkIncomingModuleAPI() {
  ignoredModuleProp('fetchSettings');
}
var ASM_CONSTS = {
  1542706: ($0) => { var str = UTF8ToString($0) + '\n\n' + 'Abort/Retry/Ignore/AlwaysIgnore? [ariA] :'; var reply = window.prompt(str, "i"); if (reply === null) { reply = "i"; } return reply.length === 1 ? reply.charCodeAt(0) : -1; },  
 1542921: () => { if (typeof(AudioContext) !== 'undefined') { return true; } else if (typeof(webkitAudioContext) !== 'undefined') { return true; } return false; },  
 1543068: () => { if ((typeof(navigator.mediaDevices) !== 'undefined') && (typeof(navigator.mediaDevices.getUserMedia) !== 'undefined')) { return true; } else if (typeof(navigator.webkitGetUserMedia) !== 'undefined') { return true; } return false; },  
 1543302: ($0) => { if(typeof(Module['SDL2']) === 'undefined') { Module['SDL2'] = {}; } var SDL2 = Module['SDL2']; if (!$0) { SDL2.audio = {}; } else { SDL2.capture = {}; } if (!SDL2.audioContext) { if (typeof(AudioContext) !== 'undefined') { SDL2.audioContext = new AudioContext(); } else if (typeof(webkitAudioContext) !== 'undefined') { SDL2.audioContext = new webkitAudioContext(); } if (SDL2.audioContext) { if ((typeof navigator.userActivation) === 'undefined') { autoResumeAudioContext(SDL2.audioContext); } } } return SDL2.audioContext === undefined ? -1 : 0; },  
 1543854: () => { var SDL2 = Module['SDL2']; return SDL2.audioContext.sampleRate; },  
 1543922: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; var have_microphone = function(stream) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); SDL2.capture.silenceTimer = undefined; SDL2.capture.silenceBuffer = undefined } SDL2.capture.mediaStreamNode = SDL2.audioContext.createMediaStreamSource(stream); SDL2.capture.scriptProcessorNode = SDL2.audioContext.createScriptProcessor($1, $0, 1); SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) { if ((SDL2 === undefined) || (SDL2.capture === undefined)) { return; } audioProcessingEvent.outputBuffer.getChannelData(0).fill(0.0); SDL2.capture.currentCaptureBuffer = audioProcessingEvent.inputBuffer; dynCall('vp', $2, [$3]); }; SDL2.capture.mediaStreamNode.connect(SDL2.capture.scriptProcessorNode); SDL2.capture.scriptProcessorNode.connect(SDL2.audioContext.destination); SDL2.capture.stream = stream; }; var no_microphone = function(error) { }; SDL2.capture.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.capture.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { SDL2.capture.currentCaptureBuffer = SDL2.capture.silenceBuffer; dynCall('vp', $2, [$3]); }; SDL2.capture.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); if ((navigator.mediaDevices !== undefined) && (navigator.mediaDevices.getUserMedia !== undefined)) { navigator.mediaDevices.getUserMedia({ audio: true, video: false }).then(have_microphone).catch(no_microphone); } else if (navigator.webkitGetUserMedia !== undefined) { navigator.webkitGetUserMedia({ audio: true, video: false }, have_microphone, no_microphone); } },  
 1545615: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; SDL2.audio.scriptProcessorNode = SDL2.audioContext['createScriptProcessor']($1, 0, $0); SDL2.audio.scriptProcessorNode['onaudioprocess'] = function (e) { if ((SDL2 === undefined) || (SDL2.audio === undefined)) { return; } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); SDL2.audio.silenceTimer = undefined; SDL2.audio.silenceBuffer = undefined; } SDL2.audio.currentOutputBuffer = e['outputBuffer']; dynCall('vp', $2, [$3]); }; SDL2.audio.scriptProcessorNode['connect'](SDL2.audioContext['destination']); if (SDL2.audioContext.state === 'suspended') { SDL2.audio.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.audio.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { if ((typeof navigator.userActivation) !== 'undefined') { if (navigator.userActivation.hasBeenActive) { SDL2.audioContext.resume(); } } SDL2.audio.currentOutputBuffer = SDL2.audio.silenceBuffer; dynCall('vp', $2, [$3]); SDL2.audio.currentOutputBuffer = undefined; }; SDL2.audio.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); } },  
 1546790: ($0, $1) => { var SDL2 = Module['SDL2']; var numChannels = SDL2.capture.currentCaptureBuffer.numberOfChannels; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.capture.currentCaptureBuffer.getChannelData(c); if (channelData.length != $1) { throw 'Web Audio capture buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } if (numChannels == 1) { for (var j = 0; j < $1; ++j) { setValue($0 + (j * 4), channelData[j], 'float'); } } else { for (var j = 0; j < $1; ++j) { setValue($0 + (((j * numChannels) + c) * 4), channelData[j], 'float'); } } } },  
 1547395: ($0, $1) => { var SDL2 = Module['SDL2']; var buf = $0 >>> 2; var numChannels = SDL2.audio.currentOutputBuffer['numberOfChannels']; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.audio.currentOutputBuffer['getChannelData'](c); if (channelData.length != $1) { throw 'Web Audio output buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } for (var j = 0; j < $1; ++j) { channelData[j] = HEAPF32[buf + (j*numChannels + c)]; } } },  
 1547884: ($0) => { var SDL2 = Module['SDL2']; if ($0) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); } if (SDL2.capture.stream !== undefined) { var tracks = SDL2.capture.stream.getAudioTracks(); for (var i = 0; i < tracks.length; i++) { SDL2.capture.stream.removeTrack(tracks[i]); } } if (SDL2.capture.scriptProcessorNode !== undefined) { SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) {}; SDL2.capture.scriptProcessorNode.disconnect(); } if (SDL2.capture.mediaStreamNode !== undefined) { SDL2.capture.mediaStreamNode.disconnect(); } SDL2.capture = undefined; } else { if (SDL2.audio.scriptProcessorNode != undefined) { SDL2.audio.scriptProcessorNode.disconnect(); } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); } SDL2.audio = undefined; } if ((SDL2.audioContext !== undefined) && (SDL2.audio === undefined) && (SDL2.capture === undefined)) { SDL2.audioContext.close(); SDL2.audioContext = undefined; } },  
 1548890: ($0, $1, $2) => { var w = $0; var h = $1; var pixels = $2; if (!Module['SDL2']) Module['SDL2'] = {}; var SDL2 = Module['SDL2']; if (SDL2.ctxCanvas !== Module['canvas']) { SDL2.ctx = Browser.createContext(Module['canvas'], false, true); SDL2.ctxCanvas = Module['canvas']; } if (SDL2.w !== w || SDL2.h !== h || SDL2.imageCtx !== SDL2.ctx) { SDL2.image = SDL2.ctx.createImageData(w, h); SDL2.w = w; SDL2.h = h; SDL2.imageCtx = SDL2.ctx; } var data = SDL2.image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = 0xff; src++; dst += 4; } } else { if (SDL2.data32Data !== data) { SDL2.data32 = new Int32Array(data.buffer); SDL2.data8 = new Uint8Array(data.buffer); SDL2.data32Data = data; } var data32 = SDL2.data32; num = data32.length; data32.set(HEAP32.subarray(src, src + num)); var data8 = SDL2.data8; var i = 3; var j = i + 4*num; if (num % 8 == 0) { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; } } else { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; } } } SDL2.ctx.putImageData(SDL2.image, 0, 0); },  
 1550356: ($0, $1, $2, $3, $4) => { var w = $0; var h = $1; var hot_x = $2; var hot_y = $3; var pixels = $4; var canvas = document.createElement("canvas"); canvas.width = w; canvas.height = h; var ctx = canvas.getContext("2d"); var image = ctx.createImageData(w, h); var data = image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = (val >> 24) & 0xff; src++; dst += 4; } } else { var data32 = new Int32Array(data.buffer); num = data32.length; data32.set(HEAP32.subarray(src, src + num)); } ctx.putImageData(image, 0, 0); var url = hot_x === 0 && hot_y === 0 ? "url(" + canvas.toDataURL() + "), auto" : "url(" + canvas.toDataURL() + ") " + hot_x + " " + hot_y + ", auto"; var urlBuf = _malloc(url.length + 1); stringToUTF8(url, urlBuf, url.length + 1); return urlBuf; },  
 1551344: ($0) => { if (Module['canvas']) { Module['canvas'].style['cursor'] = UTF8ToString($0); } },  
 1551427: () => { if (Module['canvas']) { Module['canvas'].style['cursor'] = 'none'; } },  
 1551496: () => { return window.innerWidth; },  
 1551526: () => { return window.innerHeight; },  
 1551557: () => { return Module.window ? 1 : 0; },  
 1551591: () => { var win = Module.window ? Module.window : window; var AudioContext = win.AudioContext || win.webkitAudioContext; var contextForCheck = new AudioContext(); if (!contextForCheck) { return 0; } var retValue = 0; if (self.AudioWorkletNode) { if (contextForCheck.audioWorklet.addModule) { retValue = 1; } } contextForCheck.close(); return retValue; },  
 1551939: () => { var win = Module.window ? Module.window : window; var AudioContext = win.AudioContext || win.webkitAudioContext; var infocontext = new AudioContext(); if (!infocontext) { return 0; } var inforate = infocontext.sampleRate; infocontext.close(); return inforate; },  
 1552203: () => { Module.mInputRegistered = false; },  
 1552240: () => { var win = Module.window ? Module.window : window; var AudioContext = win.AudioContext || win.webkitAudioContext; Module.context = new AudioContext(); if (!Module.context) { return 0; } Module.FMOD_JS_MixFunction = Module["cwrap"]('FMOD_JS_MixFunction', 'void', ['number']); return Module.context.sampleRate; },  
 1552552: ($0, $1) => { Module._as_script_node = Module.context.createScriptProcessor($1, 0, $0); Module["OutputWebAudio_resumeAudio"] = function() { if (Module.context && Module.mInputRegistered) { console.log('Resetting audio driver based on user input.'); Module._as_script_node.connect(Module.context.destination); Module._as_script_node.onaudioprocess = function(audioProcessingEvent) { Module._as_output_buffer = audioProcessingEvent.outputBuffer; Module.FMOD_JS_MixFunction(Module._as_output_buffer.getChannelData(0).length); }; Module.context.resume(); var win = Module.window ? Module.window : window; win.removeEventListener('click', Module.OutputWebAudio_resumeAudio, false); win.removeEventListener('touchend', Module.OutputWebAudio_resumeAudio, false); Module.mInputRegistered = false; } }; },  
 1553336: () => { if (Module.mInputRegistered) { Module.mInputRegistered = false; var win = Module.window ? Module.window : window; win.removeEventListener('click', Module["OutputWebAudio_resumeAudio"], false); win.removeEventListener('touchend', Module["OutputWebAudio_resumeAudio"], false); } },  
 1553617: () => { var win = Module.window ? Module.window : window; win.addEventListener('touchend', Module["OutputWebAudio_resumeAudio"], false); win.addEventListener('click', Module["OutputWebAudio_resumeAudio"], false); Module.mInputRegistered = true; },  
 1553858: () => { Module._as_script_node.disconnect(Module.context.destination); },  
 1553925: ($0, $1, $2, $3) => { var data = HEAPF32.subarray(($0 / 4), ($0 / 4) + ($2 * $3)); for (var channel = 0; channel < $3; channel++) { var outputData = Module._as_output_buffer.getChannelData(channel); for (var sample = 0; sample < $2; sample++) { outputData[sample+$1] = data[(sample*$3)+channel]; } } },  
 1554207: () => { Module.context.suspend(); },  
 1554237: () => { Module.context.resume(); },  
 1554266: () => { var win = Module.window ? Module.window : window; var AudioContext = win.AudioContext || win.webkitAudioContext; var infocontext = new AudioContext(); if (!infocontext) { return 0; } var inforate = infocontext.sampleRate; infocontext.close(); return inforate; },  
 1554530: () => { Module.mWorkletNode = null; Module.mModulePolling = false; Module.mModuleLoading = false; Module.mStartInterval = null; Module.mStopInterval = null; Module.mSuspendInterval = null; Module.mResumeInterval = null; Module.mWorkletNodeConnected = false; Module.mInputRegistered = false; var win = Module.window ? Module.window : window; var AudioContext = win.AudioContext || win.webkitAudioContext; Module.mContext = new AudioContext(); if (!Module.mContext) { return 0; } Module.mContext.destination.channelCount = Module.mContext.destination.maxChannelCount; return Module.mContext.destination.maxChannelCount; },  
 1555144: ($0) => { const initAddModuleRef = $0; if (!initAddModuleRef) { Module.mAddModuleRef = 0; } if (!self.AudioWorkletNode) { return -1; } if (!Module.mContext.audioWorklet.addModule) { return -2; } Module.FMOD_JS_MixerSlowpathFunction = Module["cwrap"]('FMOD_JS_MixerSlowpathFunction', 'void', []); Module.FMOD_JS_MixerFastpathFunction = Module["cwrap"]('FMOD_JS_MixerFastpathFunction', 'void', ['number']); return Module.mContext.sampleRate; },  
 1555578: ($0, $1) => { Module.mSpeakerChannelCount = $0; const bufferLength = $1; Module.mUrl = null; Module.mOutputData = null; Module.mSharedArrayBuffers = false; if (self.SharedArrayBuffer) { if (self.crossOriginIsolated) { Module.mSharedArrayBuffers = true; } } if (!Module.mSharedArrayBuffers) { const slowCodePath = new Blob( [ "class AudioProcessor extends AudioWorkletProcessor", "{", "constructor(options)", "{", "super();", "this.payload = [null, null];", "this.bufferFlag = 0;", "this.dataFlag = 0;", "this.bufferIndex = 0;", "this.bufferSize = 0;", "this.channelCount = options.outputChannelCount;", "this.port.onmessage = (event) => {", "const { data } = event;", "if (data)", "{", "this.payload[this.dataFlag] = new Float32Array(data);", "this.bufferSize = data.length / this.channelCount;", "}", "else", "{", "this.payload[this.dataFlag] = null;", "}", "this.dataFlag ^= 1;", "};", "}", "process(inputs, outputs, parameters)", "{", "const output = outputs[0];", "if (output.length === 0) return true;", "if (this.payload[this.bufferFlag]) {", "if (this.bufferIndex === 0) {", "this.port.postMessage(this.bufferSize);", "}", "const bufferSliceEnd = this.bufferSize / output[0].length;", "const sliceOffset = output[0].length * this.bufferIndex;", "for (let channel = 0; channel < output.length; ++channel) {", "const outputChannel = output[channel];", "const indexOffset = this.bufferSize * channel + sliceOffset;", "outputChannel.set(new Float32Array(this.payload[this.bufferFlag].slice(0 + indexOffset, outputChannel.length + indexOffset)));", "}", "this.bufferIndex++;", "if (this.bufferIndex === bufferSliceEnd) {", "this.bufferIndex = 0;", "this.bufferFlag ^= 1;", "}", "}", "return true;", "}", "}", "registerProcessor('audio-processor', AudioProcessor);" ], { type: 'application/javascript' }); Module.mUrl = URL.createObjectURL(slowCodePath); Module.mOutputData = new Float32Array(Module.mSpeakerChannelCount * bufferLength); } else { const fastCodePath = new Blob( [ "class AudioProcessor extends AudioWorkletProcessor", "{", "constructor(options)", "{", "super();", "this.payload = null;", "this.bufferFlag = 0;", "this.bufferIndex = 0;", "this.bufferSize = 0;", "this.bufferOffset = 0;", "this.channelCount = options.outputChannelCount;", "this.port.onmessage = (event) => {", "const { data } = event;", "if (data)", "{", "this.payload = data;", "this.bufferSize = this.payload.length / (this.channelCount * 2);", "this.bufferOffset = this.payload.length / 2;", "}", "else", "{", "this.payload = null;", "}", "};", "}", "process(inputs, outputs, parameters)", "{", "const output = outputs[0];", "if (output.length === 0) return true;", "if (this.payload) {", "if (this.bufferIndex === 0) {", "this.port.postMessage(this.bufferFlag ^ 1);", "}", "const bufferSliceEnd = this.bufferSize / output[0].length;", "const sliceOffset = output[0].length * this.bufferIndex + this.bufferOffset * this.bufferFlag;", "for (let channel = 0; channel < output.length; ++channel) {", "const outputChannel = output[channel];", "const indexOffset = this.bufferSize * channel + sliceOffset;", "outputChannel.set(new Float32Array(this.payload.slice(0 + indexOffset, outputChannel.length + indexOffset)));", "}", "this.bufferIndex++;", "if (this.bufferIndex === bufferSliceEnd) {", "this.bufferIndex = 0;", "this.bufferFlag ^= 1;", "}", "}", "return true;", "}", "}", "registerProcessor('audio-processor', AudioProcessor);" ], { type: 'application/javascript' }); Module.mUrl = URL.createObjectURL(fastCodePath); Module.mOutputData = new Float32Array(new SharedArrayBuffer(Float32Array.BYTES_PER_ELEMENT * Module.mSpeakerChannelCount * bufferLength * 2)); } Module.mModulePolling = true; Module["waitForAudioWorklet"] = function(condition, callback) { var myInterval = null; if (condition()) { myInterval = setInterval(function() { if (!condition()) { callback(); clearInterval(myInterval); } }, 50); } else { callback(); return null; } return myInterval; }; Module["OutputAudioWorklet_resumeAudio"] = function() { if (Module.mContext && Module.mInputRegistered) { console.log('Resetting audio driver based on user input.'); Module.mContext.resume(); var win = Module.window ? Module.window : window; win.removeEventListener('click', Module.OutputAudioWorklet_resumeAudio, false); win.removeEventListener('touchend', Module.OutputAudioWorklet_resumeAudio, false); Module.mInputRegistered = false; if (!Module.mModuleLoading) { Module.mModuleLoading = true; Module.mAddModuleRef++; Module.mContext.resume().then(function() { Module.mContext.audioWorklet.addModule(Module.mUrl).then(function() { if (Module.mAddModuleRef === 1) { Module.mWorkletNode = new AudioWorkletNode(Module.mContext, 'audio-processor', { 'outputChannelCount' : [Module.mSpeakerChannelCount] }); Module.mModulePolling = false; URL.revokeObjectURL(Module.mUrl); if (Module.mWorkletNode) { Module.mWorkletNode.port.postMessage(Module.mOutputData); if (Module.mSharedArrayBuffers) { Module.mWorkletNode.port.onmessage = function(event) { Module.FMOD_JS_MixerFastpathFunction(event.data); }; } else { Module.mWorkletNode.port.onmessage = function(event) { Module.FMOD_JS_MixerSlowpathFunction(); Module.mWorkletNode.port.postMessage(Module.mOutputData); }; } } else { console.log('Error when creating AudioWorkletNode: Null object'); } } Module.mAddModuleRef--; }).catch (function(err) { Module.mModulePolling = false; Module.mAddModuleRef--; console.log('Error when opening audio processor '); console.log(err) }); }).catch (function(err) { Module.mModulePolling = false; Module.mAddModuleRef--; console.log('Error with mContext.resume()'); console.log(err) }); } } }; return Module.mSharedArrayBuffers; },  
 1561233: () => { var win = Module.window ? Module.window : window; win.addEventListener('touchend', Module["OutputAudioWorklet_resumeAudio"], false); win.addEventListener('click', Module["OutputAudioWorklet_resumeAudio"], false); Module.mInputRegistered = true; },  
 1561482: () => { if (Module.mStartInterval) { clearInterval(Module.mStartInterval); } if (Module.mStopInterval) { clearInterval(Module.mStopInterval); } if (Module.mSuspendInterval) { clearInterval(Module.mSuspendInterval); } if (Module.mResumeInterval) { clearInterval(Module.mResumeInterval); } if (Module.mWorkletNode) { if (Module.mWorkletNodeConnected) { Module.mWorkletNode.disconnect(); } } if (Module.mContext) { Module.mContext.close(); } if (Module.mInputRegistered) { Module.mInputRegistered = false; var win = Module.window ? Module.window : window; win.removeEventListener('click', Module["OutputAudioWorklet_resumeAudio"], false); win.removeEventListener('touchend', Module["OutputAudioWorklet_resumeAudio"], false); } },  
 1562202: () => { Module.mStartInterval = Module["waitForAudioWorklet"](function(){ return (Module.mWorkletNode === null && Module.mModulePolling); }, function(){ if (Module.mWorkletNode) { Module.mWorkletNode.connect(Module.mContext.destination); Module.mWorkletNodeConnected = true; } }); },  
 1562479: () => { if (Module["waitForAudioWorklet"]) { Module.mStopInterval = Module["waitForAudioWorklet"](function(){ return (Module.mWorkletNode === null && Module.mModulePolling || !Module.mWorkletNodeConnected); }, function(){ if (Module.mWorkletNode) { Module.mWorkletNode.disconnect(); Module.mWorkletNodeConnected = false; } }); } },  
 1562804: ($0, $1, $2) => { const buffer = $0; const bufferLength = $1; const speakerModeChannels = $2; var data = HEAPF32.subarray((buffer / 4), (buffer / 4) + (bufferLength * speakerModeChannels)); for (var channel = 0; channel < speakerModeChannels; channel++) { const offset = channel * bufferLength; for (var sample = 0; sample < bufferLength; sample++) { Module.mOutputData[sample + offset] = data[(sample * speakerModeChannels) + channel]; } } },  
 1563231: ($0, $1, $2, $3) => { const buffer = $0; const bufferLength = $1; const speakerModeChannels = $2; const frameFlag = $3; var data = HEAPF32.subarray((buffer / 4), (buffer / 4) + (bufferLength * speakerModeChannels)); const arrayOffset = speakerModeChannels * frameFlag * bufferLength; for (var channel = 0; channel < speakerModeChannels; channel++) { const offset = channel * bufferLength + arrayOffset; for (var sample = 0; sample < bufferLength; sample++) { Module.mOutputData[sample + offset] = data[(sample * speakerModeChannels) + channel]; } } },  
 1563762: () => { if (Module.mContext) { Module.mContext.suspend(); } },  
 1563818: () => { if (Module.mContext) { Module.mContext.resume(); } },  
 1563873: () => { debugger; }
};
function canvas_get_width() { return canvas.width; }
function canvas_get_height() { return canvas.height; }
function SetupPersistentSystemJS() { try { if (!FS.analyzePath('/saves').exists) { FS.mkdir('/saves'); } FS.mount(IDBFS, {}, '/saves'); FS.syncfs(true, function(err) { if (err) console.error("Initial sync error:", err); }); } catch (e) { console.error("SetupPersistentSystemJS failed:", e); } }
function WriteFileJS(relPath,content) { const path = UTF8ToString(relPath); const data = UTF8ToString(content); try { const parts = path.split('/'); let current = ''; for (let i = 0; i < parts.length - 1; i++) { current += (i ? '/' : '') + parts[i]; if (current.length > 0 && !FS.analyzePath(current).exists) { FS.mkdir(current); } } FS.writeFile(path, data); FS.syncfs(false, function(err) { if (err) console.error("Write sync error:", err); }); } catch (e) { console.error("WriteFileJS failed:", e); } }
function release_cursor_js() { if (document.pointerLockElement === Module['canvas']) { document.exitPointerLock(); } }
function lock_cursor_js() { if (Module['canvas']) { Module['canvas'].requestPointerLock(); } }
function ImGui_ImplSDL2_EmscriptenOpenURL(url) { url = url ? UTF8ToString(url) : null; if (url) window.open(url, '_blank'); }
var wasmImports = {
  /** @export */
  ImGui_ImplSDL2_EmscriptenOpenURL,
  /** @export */
  SetupPersistentSystemJS,
  /** @export */
  WriteFileJS,
  /** @export */
  __assert_fail: ___assert_fail,
  /** @export */
  __cxa_begin_catch: ___cxa_begin_catch,
  /** @export */
  __cxa_end_catch: ___cxa_end_catch,
  /** @export */
  __cxa_find_matching_catch_2: ___cxa_find_matching_catch_2,
  /** @export */
  __cxa_find_matching_catch_3: ___cxa_find_matching_catch_3,
  /** @export */
  __cxa_rethrow: ___cxa_rethrow,
  /** @export */
  __cxa_throw: ___cxa_throw,
  /** @export */
  __cxa_uncaught_exceptions: ___cxa_uncaught_exceptions,
  /** @export */
  __resumeException: ___resumeException,
  /** @export */
  __syscall_chdir: ___syscall_chdir,
  /** @export */
  __syscall_dup3: ___syscall_dup3,
  /** @export */
  __syscall_fcntl64: ___syscall_fcntl64,
  /** @export */
  __syscall_fstat64: ___syscall_fstat64,
  /** @export */
  __syscall_ftruncate64: ___syscall_ftruncate64,
  /** @export */
  __syscall_getcwd: ___syscall_getcwd,
  /** @export */
  __syscall_getdents64: ___syscall_getdents64,
  /** @export */
  __syscall_ioctl: ___syscall_ioctl,
  /** @export */
  __syscall_lstat64: ___syscall_lstat64,
  /** @export */
  __syscall_mkdirat: ___syscall_mkdirat,
  /** @export */
  __syscall_newfstatat: ___syscall_newfstatat,
  /** @export */
  __syscall_openat: ___syscall_openat,
  /** @export */
  __syscall_readlinkat: ___syscall_readlinkat,
  /** @export */
  __syscall_rmdir: ___syscall_rmdir,
  /** @export */
  __syscall_stat64: ___syscall_stat64,
  /** @export */
  __syscall_unlinkat: ___syscall_unlinkat,
  /** @export */
  _abort_js: __abort_js,
  /** @export */
  _emscripten_throw_longjmp: __emscripten_throw_longjmp,
  /** @export */
  _localtime_js: __localtime_js,
  /** @export */
  _mktime_js: __mktime_js,
  /** @export */
  _mmap_js: __mmap_js,
  /** @export */
  _munmap_js: __munmap_js,
  /** @export */
  _tzset_js: __tzset_js,
  /** @export */
  alBufferData: _alBufferData,
  /** @export */
  alDeleteBuffers: _alDeleteBuffers,
  /** @export */
  alDeleteSources: _alDeleteSources,
  /** @export */
  alDistanceModel: _alDistanceModel,
  /** @export */
  alGenBuffers: _alGenBuffers,
  /** @export */
  alGenSources: _alGenSources,
  /** @export */
  alGetError: _alGetError,
  /** @export */
  alGetListenerfv: _alGetListenerfv,
  /** @export */
  alGetSourcei: _alGetSourcei,
  /** @export */
  alGetString: _alGetString,
  /** @export */
  alIsExtensionPresent: _alIsExtensionPresent,
  /** @export */
  alListener3f: _alListener3f,
  /** @export */
  alListenerfv: _alListenerfv,
  /** @export */
  alSource3f: _alSource3f,
  /** @export */
  alSourcePlay: _alSourcePlay,
  /** @export */
  alSourceQueueBuffers: _alSourceQueueBuffers,
  /** @export */
  alSourceStop: _alSourceStop,
  /** @export */
  alSourceUnqueueBuffers: _alSourceUnqueueBuffers,
  /** @export */
  alSourcef: _alSourcef,
  /** @export */
  alSourcei: _alSourcei,
  /** @export */
  alcCloseDevice: _alcCloseDevice,
  /** @export */
  alcCreateContext: _alcCreateContext,
  /** @export */
  alcDestroyContext: _alcDestroyContext,
  /** @export */
  alcGetContextsDevice: _alcGetContextsDevice,
  /** @export */
  alcGetCurrentContext: _alcGetCurrentContext,
  /** @export */
  alcGetIntegerv: _alcGetIntegerv,
  /** @export */
  alcIsExtensionPresent: _alcIsExtensionPresent,
  /** @export */
  alcMakeContextCurrent: _alcMakeContextCurrent,
  /** @export */
  alcOpenDevice: _alcOpenDevice,
  /** @export */
  clock_time_get: _clock_time_get,
  /** @export */
  eglBindAPI: _eglBindAPI,
  /** @export */
  eglChooseConfig: _eglChooseConfig,
  /** @export */
  eglCreateContext: _eglCreateContext,
  /** @export */
  eglCreateWindowSurface: _eglCreateWindowSurface,
  /** @export */
  eglDestroyContext: _eglDestroyContext,
  /** @export */
  eglDestroySurface: _eglDestroySurface,
  /** @export */
  eglGetConfigAttrib: _eglGetConfigAttrib,
  /** @export */
  eglGetDisplay: _eglGetDisplay,
  /** @export */
  eglGetError: _eglGetError,
  /** @export */
  eglInitialize: _eglInitialize,
  /** @export */
  eglMakeCurrent: _eglMakeCurrent,
  /** @export */
  eglQueryString: _eglQueryString,
  /** @export */
  eglSwapBuffers: _eglSwapBuffers,
  /** @export */
  eglSwapInterval: _eglSwapInterval,
  /** @export */
  eglTerminate: _eglTerminate,
  /** @export */
  eglWaitGL: _eglWaitGL,
  /** @export */
  eglWaitNative: _eglWaitNative,
  /** @export */
  emscripten_asm_const_int: _emscripten_asm_const_int,
  /** @export */
  emscripten_asm_const_int_sync_on_main_thread: _emscripten_asm_const_int_sync_on_main_thread,
  /** @export */
  emscripten_asm_const_ptr_sync_on_main_thread: _emscripten_asm_const_ptr_sync_on_main_thread,
  /** @export */
  emscripten_date_now: _emscripten_date_now,
  /** @export */
  emscripten_err: _emscripten_err,
  /** @export */
  emscripten_exit_fullscreen: _emscripten_exit_fullscreen,
  /** @export */
  emscripten_exit_pointerlock: _emscripten_exit_pointerlock,
  /** @export */
  emscripten_fetch_free: _emscripten_fetch_free,
  /** @export */
  emscripten_get_canvas_element_size: _emscripten_get_canvas_element_size,
  /** @export */
  emscripten_get_device_pixel_ratio: _emscripten_get_device_pixel_ratio,
  /** @export */
  emscripten_get_element_css_size: _emscripten_get_element_css_size,
  /** @export */
  emscripten_get_gamepad_status: _emscripten_get_gamepad_status,
  /** @export */
  emscripten_get_heap_max: _emscripten_get_heap_max,
  /** @export */
  emscripten_get_now: _emscripten_get_now,
  /** @export */
  emscripten_get_num_gamepads: _emscripten_get_num_gamepads,
  /** @export */
  emscripten_get_screen_size: _emscripten_get_screen_size,
  /** @export */
  emscripten_glActiveTexture: _emscripten_glActiveTexture,
  /** @export */
  emscripten_glAttachShader: _emscripten_glAttachShader,
  /** @export */
  emscripten_glBeginQuery: _emscripten_glBeginQuery,
  /** @export */
  emscripten_glBeginQueryEXT: _emscripten_glBeginQueryEXT,
  /** @export */
  emscripten_glBeginTransformFeedback: _emscripten_glBeginTransformFeedback,
  /** @export */
  emscripten_glBindAttribLocation: _emscripten_glBindAttribLocation,
  /** @export */
  emscripten_glBindBuffer: _emscripten_glBindBuffer,
  /** @export */
  emscripten_glBindBufferBase: _emscripten_glBindBufferBase,
  /** @export */
  emscripten_glBindBufferRange: _emscripten_glBindBufferRange,
  /** @export */
  emscripten_glBindFramebuffer: _emscripten_glBindFramebuffer,
  /** @export */
  emscripten_glBindRenderbuffer: _emscripten_glBindRenderbuffer,
  /** @export */
  emscripten_glBindSampler: _emscripten_glBindSampler,
  /** @export */
  emscripten_glBindTexture: _emscripten_glBindTexture,
  /** @export */
  emscripten_glBindTransformFeedback: _emscripten_glBindTransformFeedback,
  /** @export */
  emscripten_glBindVertexArray: _emscripten_glBindVertexArray,
  /** @export */
  emscripten_glBindVertexArrayOES: _emscripten_glBindVertexArrayOES,
  /** @export */
  emscripten_glBlendColor: _emscripten_glBlendColor,
  /** @export */
  emscripten_glBlendEquation: _emscripten_glBlendEquation,
  /** @export */
  emscripten_glBlendEquationSeparate: _emscripten_glBlendEquationSeparate,
  /** @export */
  emscripten_glBlendFunc: _emscripten_glBlendFunc,
  /** @export */
  emscripten_glBlendFuncSeparate: _emscripten_glBlendFuncSeparate,
  /** @export */
  emscripten_glBlitFramebuffer: _emscripten_glBlitFramebuffer,
  /** @export */
  emscripten_glBufferData: _emscripten_glBufferData,
  /** @export */
  emscripten_glBufferSubData: _emscripten_glBufferSubData,
  /** @export */
  emscripten_glCheckFramebufferStatus: _emscripten_glCheckFramebufferStatus,
  /** @export */
  emscripten_glClear: _emscripten_glClear,
  /** @export */
  emscripten_glClearBufferfi: _emscripten_glClearBufferfi,
  /** @export */
  emscripten_glClearBufferfv: _emscripten_glClearBufferfv,
  /** @export */
  emscripten_glClearBufferiv: _emscripten_glClearBufferiv,
  /** @export */
  emscripten_glClearBufferuiv: _emscripten_glClearBufferuiv,
  /** @export */
  emscripten_glClearColor: _emscripten_glClearColor,
  /** @export */
  emscripten_glClearDepthf: _emscripten_glClearDepthf,
  /** @export */
  emscripten_glClearStencil: _emscripten_glClearStencil,
  /** @export */
  emscripten_glClientWaitSync: _emscripten_glClientWaitSync,
  /** @export */
  emscripten_glClipControlEXT: _emscripten_glClipControlEXT,
  /** @export */
  emscripten_glColorMask: _emscripten_glColorMask,
  /** @export */
  emscripten_glCompileShader: _emscripten_glCompileShader,
  /** @export */
  emscripten_glCompressedTexImage2D: _emscripten_glCompressedTexImage2D,
  /** @export */
  emscripten_glCompressedTexImage3D: _emscripten_glCompressedTexImage3D,
  /** @export */
  emscripten_glCompressedTexSubImage2D: _emscripten_glCompressedTexSubImage2D,
  /** @export */
  emscripten_glCompressedTexSubImage3D: _emscripten_glCompressedTexSubImage3D,
  /** @export */
  emscripten_glCopyBufferSubData: _emscripten_glCopyBufferSubData,
  /** @export */
  emscripten_glCopyTexImage2D: _emscripten_glCopyTexImage2D,
  /** @export */
  emscripten_glCopyTexSubImage2D: _emscripten_glCopyTexSubImage2D,
  /** @export */
  emscripten_glCopyTexSubImage3D: _emscripten_glCopyTexSubImage3D,
  /** @export */
  emscripten_glCreateProgram: _emscripten_glCreateProgram,
  /** @export */
  emscripten_glCreateShader: _emscripten_glCreateShader,
  /** @export */
  emscripten_glCullFace: _emscripten_glCullFace,
  /** @export */
  emscripten_glDeleteBuffers: _emscripten_glDeleteBuffers,
  /** @export */
  emscripten_glDeleteFramebuffers: _emscripten_glDeleteFramebuffers,
  /** @export */
  emscripten_glDeleteProgram: _emscripten_glDeleteProgram,
  /** @export */
  emscripten_glDeleteQueries: _emscripten_glDeleteQueries,
  /** @export */
  emscripten_glDeleteQueriesEXT: _emscripten_glDeleteQueriesEXT,
  /** @export */
  emscripten_glDeleteRenderbuffers: _emscripten_glDeleteRenderbuffers,
  /** @export */
  emscripten_glDeleteSamplers: _emscripten_glDeleteSamplers,
  /** @export */
  emscripten_glDeleteShader: _emscripten_glDeleteShader,
  /** @export */
  emscripten_glDeleteSync: _emscripten_glDeleteSync,
  /** @export */
  emscripten_glDeleteTextures: _emscripten_glDeleteTextures,
  /** @export */
  emscripten_glDeleteTransformFeedbacks: _emscripten_glDeleteTransformFeedbacks,
  /** @export */
  emscripten_glDeleteVertexArrays: _emscripten_glDeleteVertexArrays,
  /** @export */
  emscripten_glDeleteVertexArraysOES: _emscripten_glDeleteVertexArraysOES,
  /** @export */
  emscripten_glDepthFunc: _emscripten_glDepthFunc,
  /** @export */
  emscripten_glDepthMask: _emscripten_glDepthMask,
  /** @export */
  emscripten_glDepthRangef: _emscripten_glDepthRangef,
  /** @export */
  emscripten_glDetachShader: _emscripten_glDetachShader,
  /** @export */
  emscripten_glDisable: _emscripten_glDisable,
  /** @export */
  emscripten_glDisableVertexAttribArray: _emscripten_glDisableVertexAttribArray,
  /** @export */
  emscripten_glDrawArrays: _emscripten_glDrawArrays,
  /** @export */
  emscripten_glDrawArraysInstanced: _emscripten_glDrawArraysInstanced,
  /** @export */
  emscripten_glDrawArraysInstancedANGLE: _emscripten_glDrawArraysInstancedANGLE,
  /** @export */
  emscripten_glDrawArraysInstancedARB: _emscripten_glDrawArraysInstancedARB,
  /** @export */
  emscripten_glDrawArraysInstancedEXT: _emscripten_glDrawArraysInstancedEXT,
  /** @export */
  emscripten_glDrawArraysInstancedNV: _emscripten_glDrawArraysInstancedNV,
  /** @export */
  emscripten_glDrawBuffers: _emscripten_glDrawBuffers,
  /** @export */
  emscripten_glDrawBuffersEXT: _emscripten_glDrawBuffersEXT,
  /** @export */
  emscripten_glDrawBuffersWEBGL: _emscripten_glDrawBuffersWEBGL,
  /** @export */
  emscripten_glDrawElements: _emscripten_glDrawElements,
  /** @export */
  emscripten_glDrawElementsInstanced: _emscripten_glDrawElementsInstanced,
  /** @export */
  emscripten_glDrawElementsInstancedANGLE: _emscripten_glDrawElementsInstancedANGLE,
  /** @export */
  emscripten_glDrawElementsInstancedARB: _emscripten_glDrawElementsInstancedARB,
  /** @export */
  emscripten_glDrawElementsInstancedEXT: _emscripten_glDrawElementsInstancedEXT,
  /** @export */
  emscripten_glDrawElementsInstancedNV: _emscripten_glDrawElementsInstancedNV,
  /** @export */
  emscripten_glDrawRangeElements: _emscripten_glDrawRangeElements,
  /** @export */
  emscripten_glEnable: _emscripten_glEnable,
  /** @export */
  emscripten_glEnableVertexAttribArray: _emscripten_glEnableVertexAttribArray,
  /** @export */
  emscripten_glEndQuery: _emscripten_glEndQuery,
  /** @export */
  emscripten_glEndQueryEXT: _emscripten_glEndQueryEXT,
  /** @export */
  emscripten_glEndTransformFeedback: _emscripten_glEndTransformFeedback,
  /** @export */
  emscripten_glFenceSync: _emscripten_glFenceSync,
  /** @export */
  emscripten_glFinish: _emscripten_glFinish,
  /** @export */
  emscripten_glFlush: _emscripten_glFlush,
  /** @export */
  emscripten_glFramebufferRenderbuffer: _emscripten_glFramebufferRenderbuffer,
  /** @export */
  emscripten_glFramebufferTexture2D: _emscripten_glFramebufferTexture2D,
  /** @export */
  emscripten_glFramebufferTextureLayer: _emscripten_glFramebufferTextureLayer,
  /** @export */
  emscripten_glFrontFace: _emscripten_glFrontFace,
  /** @export */
  emscripten_glGenBuffers: _emscripten_glGenBuffers,
  /** @export */
  emscripten_glGenFramebuffers: _emscripten_glGenFramebuffers,
  /** @export */
  emscripten_glGenQueries: _emscripten_glGenQueries,
  /** @export */
  emscripten_glGenQueriesEXT: _emscripten_glGenQueriesEXT,
  /** @export */
  emscripten_glGenRenderbuffers: _emscripten_glGenRenderbuffers,
  /** @export */
  emscripten_glGenSamplers: _emscripten_glGenSamplers,
  /** @export */
  emscripten_glGenTextures: _emscripten_glGenTextures,
  /** @export */
  emscripten_glGenTransformFeedbacks: _emscripten_glGenTransformFeedbacks,
  /** @export */
  emscripten_glGenVertexArrays: _emscripten_glGenVertexArrays,
  /** @export */
  emscripten_glGenVertexArraysOES: _emscripten_glGenVertexArraysOES,
  /** @export */
  emscripten_glGenerateMipmap: _emscripten_glGenerateMipmap,
  /** @export */
  emscripten_glGetActiveAttrib: _emscripten_glGetActiveAttrib,
  /** @export */
  emscripten_glGetActiveUniform: _emscripten_glGetActiveUniform,
  /** @export */
  emscripten_glGetActiveUniformBlockName: _emscripten_glGetActiveUniformBlockName,
  /** @export */
  emscripten_glGetActiveUniformBlockiv: _emscripten_glGetActiveUniformBlockiv,
  /** @export */
  emscripten_glGetActiveUniformsiv: _emscripten_glGetActiveUniformsiv,
  /** @export */
  emscripten_glGetAttachedShaders: _emscripten_glGetAttachedShaders,
  /** @export */
  emscripten_glGetAttribLocation: _emscripten_glGetAttribLocation,
  /** @export */
  emscripten_glGetBooleanv: _emscripten_glGetBooleanv,
  /** @export */
  emscripten_glGetBufferParameteri64v: _emscripten_glGetBufferParameteri64v,
  /** @export */
  emscripten_glGetBufferParameteriv: _emscripten_glGetBufferParameteriv,
  /** @export */
  emscripten_glGetError: _emscripten_glGetError,
  /** @export */
  emscripten_glGetFloatv: _emscripten_glGetFloatv,
  /** @export */
  emscripten_glGetFragDataLocation: _emscripten_glGetFragDataLocation,
  /** @export */
  emscripten_glGetFramebufferAttachmentParameteriv: _emscripten_glGetFramebufferAttachmentParameteriv,
  /** @export */
  emscripten_glGetInteger64i_v: _emscripten_glGetInteger64i_v,
  /** @export */
  emscripten_glGetInteger64v: _emscripten_glGetInteger64v,
  /** @export */
  emscripten_glGetIntegeri_v: _emscripten_glGetIntegeri_v,
  /** @export */
  emscripten_glGetIntegerv: _emscripten_glGetIntegerv,
  /** @export */
  emscripten_glGetInternalformativ: _emscripten_glGetInternalformativ,
  /** @export */
  emscripten_glGetProgramBinary: _emscripten_glGetProgramBinary,
  /** @export */
  emscripten_glGetProgramInfoLog: _emscripten_glGetProgramInfoLog,
  /** @export */
  emscripten_glGetProgramiv: _emscripten_glGetProgramiv,
  /** @export */
  emscripten_glGetQueryObjecti64vEXT: _emscripten_glGetQueryObjecti64vEXT,
  /** @export */
  emscripten_glGetQueryObjectivEXT: _emscripten_glGetQueryObjectivEXT,
  /** @export */
  emscripten_glGetQueryObjectui64vEXT: _emscripten_glGetQueryObjectui64vEXT,
  /** @export */
  emscripten_glGetQueryObjectuiv: _emscripten_glGetQueryObjectuiv,
  /** @export */
  emscripten_glGetQueryObjectuivEXT: _emscripten_glGetQueryObjectuivEXT,
  /** @export */
  emscripten_glGetQueryiv: _emscripten_glGetQueryiv,
  /** @export */
  emscripten_glGetQueryivEXT: _emscripten_glGetQueryivEXT,
  /** @export */
  emscripten_glGetRenderbufferParameteriv: _emscripten_glGetRenderbufferParameteriv,
  /** @export */
  emscripten_glGetSamplerParameterfv: _emscripten_glGetSamplerParameterfv,
  /** @export */
  emscripten_glGetSamplerParameteriv: _emscripten_glGetSamplerParameteriv,
  /** @export */
  emscripten_glGetShaderInfoLog: _emscripten_glGetShaderInfoLog,
  /** @export */
  emscripten_glGetShaderPrecisionFormat: _emscripten_glGetShaderPrecisionFormat,
  /** @export */
  emscripten_glGetShaderSource: _emscripten_glGetShaderSource,
  /** @export */
  emscripten_glGetShaderiv: _emscripten_glGetShaderiv,
  /** @export */
  emscripten_glGetString: _emscripten_glGetString,
  /** @export */
  emscripten_glGetStringi: _emscripten_glGetStringi,
  /** @export */
  emscripten_glGetSynciv: _emscripten_glGetSynciv,
  /** @export */
  emscripten_glGetTexParameterfv: _emscripten_glGetTexParameterfv,
  /** @export */
  emscripten_glGetTexParameteriv: _emscripten_glGetTexParameteriv,
  /** @export */
  emscripten_glGetTransformFeedbackVarying: _emscripten_glGetTransformFeedbackVarying,
  /** @export */
  emscripten_glGetUniformBlockIndex: _emscripten_glGetUniformBlockIndex,
  /** @export */
  emscripten_glGetUniformIndices: _emscripten_glGetUniformIndices,
  /** @export */
  emscripten_glGetUniformLocation: _emscripten_glGetUniformLocation,
  /** @export */
  emscripten_glGetUniformfv: _emscripten_glGetUniformfv,
  /** @export */
  emscripten_glGetUniformiv: _emscripten_glGetUniformiv,
  /** @export */
  emscripten_glGetUniformuiv: _emscripten_glGetUniformuiv,
  /** @export */
  emscripten_glGetVertexAttribIiv: _emscripten_glGetVertexAttribIiv,
  /** @export */
  emscripten_glGetVertexAttribIuiv: _emscripten_glGetVertexAttribIuiv,
  /** @export */
  emscripten_glGetVertexAttribPointerv: _emscripten_glGetVertexAttribPointerv,
  /** @export */
  emscripten_glGetVertexAttribfv: _emscripten_glGetVertexAttribfv,
  /** @export */
  emscripten_glGetVertexAttribiv: _emscripten_glGetVertexAttribiv,
  /** @export */
  emscripten_glHint: _emscripten_glHint,
  /** @export */
  emscripten_glInvalidateFramebuffer: _emscripten_glInvalidateFramebuffer,
  /** @export */
  emscripten_glInvalidateSubFramebuffer: _emscripten_glInvalidateSubFramebuffer,
  /** @export */
  emscripten_glIsBuffer: _emscripten_glIsBuffer,
  /** @export */
  emscripten_glIsEnabled: _emscripten_glIsEnabled,
  /** @export */
  emscripten_glIsFramebuffer: _emscripten_glIsFramebuffer,
  /** @export */
  emscripten_glIsProgram: _emscripten_glIsProgram,
  /** @export */
  emscripten_glIsQuery: _emscripten_glIsQuery,
  /** @export */
  emscripten_glIsQueryEXT: _emscripten_glIsQueryEXT,
  /** @export */
  emscripten_glIsRenderbuffer: _emscripten_glIsRenderbuffer,
  /** @export */
  emscripten_glIsSampler: _emscripten_glIsSampler,
  /** @export */
  emscripten_glIsShader: _emscripten_glIsShader,
  /** @export */
  emscripten_glIsSync: _emscripten_glIsSync,
  /** @export */
  emscripten_glIsTexture: _emscripten_glIsTexture,
  /** @export */
  emscripten_glIsTransformFeedback: _emscripten_glIsTransformFeedback,
  /** @export */
  emscripten_glIsVertexArray: _emscripten_glIsVertexArray,
  /** @export */
  emscripten_glIsVertexArrayOES: _emscripten_glIsVertexArrayOES,
  /** @export */
  emscripten_glLineWidth: _emscripten_glLineWidth,
  /** @export */
  emscripten_glLinkProgram: _emscripten_glLinkProgram,
  /** @export */
  emscripten_glPauseTransformFeedback: _emscripten_glPauseTransformFeedback,
  /** @export */
  emscripten_glPixelStorei: _emscripten_glPixelStorei,
  /** @export */
  emscripten_glPolygonModeWEBGL: _emscripten_glPolygonModeWEBGL,
  /** @export */
  emscripten_glPolygonOffset: _emscripten_glPolygonOffset,
  /** @export */
  emscripten_glPolygonOffsetClampEXT: _emscripten_glPolygonOffsetClampEXT,
  /** @export */
  emscripten_glProgramBinary: _emscripten_glProgramBinary,
  /** @export */
  emscripten_glProgramParameteri: _emscripten_glProgramParameteri,
  /** @export */
  emscripten_glQueryCounterEXT: _emscripten_glQueryCounterEXT,
  /** @export */
  emscripten_glReadBuffer: _emscripten_glReadBuffer,
  /** @export */
  emscripten_glReadPixels: _emscripten_glReadPixels,
  /** @export */
  emscripten_glReleaseShaderCompiler: _emscripten_glReleaseShaderCompiler,
  /** @export */
  emscripten_glRenderbufferStorage: _emscripten_glRenderbufferStorage,
  /** @export */
  emscripten_glRenderbufferStorageMultisample: _emscripten_glRenderbufferStorageMultisample,
  /** @export */
  emscripten_glResumeTransformFeedback: _emscripten_glResumeTransformFeedback,
  /** @export */
  emscripten_glSampleCoverage: _emscripten_glSampleCoverage,
  /** @export */
  emscripten_glSamplerParameterf: _emscripten_glSamplerParameterf,
  /** @export */
  emscripten_glSamplerParameterfv: _emscripten_glSamplerParameterfv,
  /** @export */
  emscripten_glSamplerParameteri: _emscripten_glSamplerParameteri,
  /** @export */
  emscripten_glSamplerParameteriv: _emscripten_glSamplerParameteriv,
  /** @export */
  emscripten_glScissor: _emscripten_glScissor,
  /** @export */
  emscripten_glShaderBinary: _emscripten_glShaderBinary,
  /** @export */
  emscripten_glShaderSource: _emscripten_glShaderSource,
  /** @export */
  emscripten_glStencilFunc: _emscripten_glStencilFunc,
  /** @export */
  emscripten_glStencilFuncSeparate: _emscripten_glStencilFuncSeparate,
  /** @export */
  emscripten_glStencilMask: _emscripten_glStencilMask,
  /** @export */
  emscripten_glStencilMaskSeparate: _emscripten_glStencilMaskSeparate,
  /** @export */
  emscripten_glStencilOp: _emscripten_glStencilOp,
  /** @export */
  emscripten_glStencilOpSeparate: _emscripten_glStencilOpSeparate,
  /** @export */
  emscripten_glTexImage2D: _emscripten_glTexImage2D,
  /** @export */
  emscripten_glTexImage3D: _emscripten_glTexImage3D,
  /** @export */
  emscripten_glTexParameterf: _emscripten_glTexParameterf,
  /** @export */
  emscripten_glTexParameterfv: _emscripten_glTexParameterfv,
  /** @export */
  emscripten_glTexParameteri: _emscripten_glTexParameteri,
  /** @export */
  emscripten_glTexParameteriv: _emscripten_glTexParameteriv,
  /** @export */
  emscripten_glTexStorage2D: _emscripten_glTexStorage2D,
  /** @export */
  emscripten_glTexStorage3D: _emscripten_glTexStorage3D,
  /** @export */
  emscripten_glTexSubImage2D: _emscripten_glTexSubImage2D,
  /** @export */
  emscripten_glTexSubImage3D: _emscripten_glTexSubImage3D,
  /** @export */
  emscripten_glTransformFeedbackVaryings: _emscripten_glTransformFeedbackVaryings,
  /** @export */
  emscripten_glUniform1f: _emscripten_glUniform1f,
  /** @export */
  emscripten_glUniform1fv: _emscripten_glUniform1fv,
  /** @export */
  emscripten_glUniform1i: _emscripten_glUniform1i,
  /** @export */
  emscripten_glUniform1iv: _emscripten_glUniform1iv,
  /** @export */
  emscripten_glUniform1ui: _emscripten_glUniform1ui,
  /** @export */
  emscripten_glUniform1uiv: _emscripten_glUniform1uiv,
  /** @export */
  emscripten_glUniform2f: _emscripten_glUniform2f,
  /** @export */
  emscripten_glUniform2fv: _emscripten_glUniform2fv,
  /** @export */
  emscripten_glUniform2i: _emscripten_glUniform2i,
  /** @export */
  emscripten_glUniform2iv: _emscripten_glUniform2iv,
  /** @export */
  emscripten_glUniform2ui: _emscripten_glUniform2ui,
  /** @export */
  emscripten_glUniform2uiv: _emscripten_glUniform2uiv,
  /** @export */
  emscripten_glUniform3f: _emscripten_glUniform3f,
  /** @export */
  emscripten_glUniform3fv: _emscripten_glUniform3fv,
  /** @export */
  emscripten_glUniform3i: _emscripten_glUniform3i,
  /** @export */
  emscripten_glUniform3iv: _emscripten_glUniform3iv,
  /** @export */
  emscripten_glUniform3ui: _emscripten_glUniform3ui,
  /** @export */
  emscripten_glUniform3uiv: _emscripten_glUniform3uiv,
  /** @export */
  emscripten_glUniform4f: _emscripten_glUniform4f,
  /** @export */
  emscripten_glUniform4fv: _emscripten_glUniform4fv,
  /** @export */
  emscripten_glUniform4i: _emscripten_glUniform4i,
  /** @export */
  emscripten_glUniform4iv: _emscripten_glUniform4iv,
  /** @export */
  emscripten_glUniform4ui: _emscripten_glUniform4ui,
  /** @export */
  emscripten_glUniform4uiv: _emscripten_glUniform4uiv,
  /** @export */
  emscripten_glUniformBlockBinding: _emscripten_glUniformBlockBinding,
  /** @export */
  emscripten_glUniformMatrix2fv: _emscripten_glUniformMatrix2fv,
  /** @export */
  emscripten_glUniformMatrix2x3fv: _emscripten_glUniformMatrix2x3fv,
  /** @export */
  emscripten_glUniformMatrix2x4fv: _emscripten_glUniformMatrix2x4fv,
  /** @export */
  emscripten_glUniformMatrix3fv: _emscripten_glUniformMatrix3fv,
  /** @export */
  emscripten_glUniformMatrix3x2fv: _emscripten_glUniformMatrix3x2fv,
  /** @export */
  emscripten_glUniformMatrix3x4fv: _emscripten_glUniformMatrix3x4fv,
  /** @export */
  emscripten_glUniformMatrix4fv: _emscripten_glUniformMatrix4fv,
  /** @export */
  emscripten_glUniformMatrix4x2fv: _emscripten_glUniformMatrix4x2fv,
  /** @export */
  emscripten_glUniformMatrix4x3fv: _emscripten_glUniformMatrix4x3fv,
  /** @export */
  emscripten_glUseProgram: _emscripten_glUseProgram,
  /** @export */
  emscripten_glValidateProgram: _emscripten_glValidateProgram,
  /** @export */
  emscripten_glVertexAttrib1f: _emscripten_glVertexAttrib1f,
  /** @export */
  emscripten_glVertexAttrib1fv: _emscripten_glVertexAttrib1fv,
  /** @export */
  emscripten_glVertexAttrib2f: _emscripten_glVertexAttrib2f,
  /** @export */
  emscripten_glVertexAttrib2fv: _emscripten_glVertexAttrib2fv,
  /** @export */
  emscripten_glVertexAttrib3f: _emscripten_glVertexAttrib3f,
  /** @export */
  emscripten_glVertexAttrib3fv: _emscripten_glVertexAttrib3fv,
  /** @export */
  emscripten_glVertexAttrib4f: _emscripten_glVertexAttrib4f,
  /** @export */
  emscripten_glVertexAttrib4fv: _emscripten_glVertexAttrib4fv,
  /** @export */
  emscripten_glVertexAttribDivisor: _emscripten_glVertexAttribDivisor,
  /** @export */
  emscripten_glVertexAttribDivisorANGLE: _emscripten_glVertexAttribDivisorANGLE,
  /** @export */
  emscripten_glVertexAttribDivisorARB: _emscripten_glVertexAttribDivisorARB,
  /** @export */
  emscripten_glVertexAttribDivisorEXT: _emscripten_glVertexAttribDivisorEXT,
  /** @export */
  emscripten_glVertexAttribDivisorNV: _emscripten_glVertexAttribDivisorNV,
  /** @export */
  emscripten_glVertexAttribI4i: _emscripten_glVertexAttribI4i,
  /** @export */
  emscripten_glVertexAttribI4iv: _emscripten_glVertexAttribI4iv,
  /** @export */
  emscripten_glVertexAttribI4ui: _emscripten_glVertexAttribI4ui,
  /** @export */
  emscripten_glVertexAttribI4uiv: _emscripten_glVertexAttribI4uiv,
  /** @export */
  emscripten_glVertexAttribIPointer: _emscripten_glVertexAttribIPointer,
  /** @export */
  emscripten_glVertexAttribPointer: _emscripten_glVertexAttribPointer,
  /** @export */
  emscripten_glViewport: _emscripten_glViewport,
  /** @export */
  emscripten_glWaitSync: _emscripten_glWaitSync,
  /** @export */
  emscripten_has_asyncify: _emscripten_has_asyncify,
  /** @export */
  emscripten_is_main_browser_thread: _emscripten_is_main_browser_thread,
  /** @export */
  emscripten_log: _emscripten_log,
  /** @export */
  emscripten_request_fullscreen_strategy: _emscripten_request_fullscreen_strategy,
  /** @export */
  emscripten_request_pointerlock: _emscripten_request_pointerlock,
  /** @export */
  emscripten_resize_heap: _emscripten_resize_heap,
  /** @export */
  emscripten_sample_gamepad_data: _emscripten_sample_gamepad_data,
  /** @export */
  emscripten_set_beforeunload_callback_on_thread: _emscripten_set_beforeunload_callback_on_thread,
  /** @export */
  emscripten_set_blur_callback_on_thread: _emscripten_set_blur_callback_on_thread,
  /** @export */
  emscripten_set_canvas_element_size: _emscripten_set_canvas_element_size,
  /** @export */
  emscripten_set_element_css_size: _emscripten_set_element_css_size,
  /** @export */
  emscripten_set_focus_callback_on_thread: _emscripten_set_focus_callback_on_thread,
  /** @export */
  emscripten_set_fullscreenchange_callback_on_thread: _emscripten_set_fullscreenchange_callback_on_thread,
  /** @export */
  emscripten_set_gamepadconnected_callback_on_thread: _emscripten_set_gamepadconnected_callback_on_thread,
  /** @export */
  emscripten_set_gamepaddisconnected_callback_on_thread: _emscripten_set_gamepaddisconnected_callback_on_thread,
  /** @export */
  emscripten_set_keydown_callback_on_thread: _emscripten_set_keydown_callback_on_thread,
  /** @export */
  emscripten_set_keypress_callback_on_thread: _emscripten_set_keypress_callback_on_thread,
  /** @export */
  emscripten_set_keyup_callback_on_thread: _emscripten_set_keyup_callback_on_thread,
  /** @export */
  emscripten_set_main_loop: _emscripten_set_main_loop,
  /** @export */
  emscripten_set_mousedown_callback_on_thread: _emscripten_set_mousedown_callback_on_thread,
  /** @export */
  emscripten_set_mouseenter_callback_on_thread: _emscripten_set_mouseenter_callback_on_thread,
  /** @export */
  emscripten_set_mouseleave_callback_on_thread: _emscripten_set_mouseleave_callback_on_thread,
  /** @export */
  emscripten_set_mousemove_callback_on_thread: _emscripten_set_mousemove_callback_on_thread,
  /** @export */
  emscripten_set_mouseup_callback_on_thread: _emscripten_set_mouseup_callback_on_thread,
  /** @export */
  emscripten_set_pointerlockchange_callback_on_thread: _emscripten_set_pointerlockchange_callback_on_thread,
  /** @export */
  emscripten_set_resize_callback_on_thread: _emscripten_set_resize_callback_on_thread,
  /** @export */
  emscripten_set_touchcancel_callback_on_thread: _emscripten_set_touchcancel_callback_on_thread,
  /** @export */
  emscripten_set_touchend_callback_on_thread: _emscripten_set_touchend_callback_on_thread,
  /** @export */
  emscripten_set_touchmove_callback_on_thread: _emscripten_set_touchmove_callback_on_thread,
  /** @export */
  emscripten_set_touchstart_callback_on_thread: _emscripten_set_touchstart_callback_on_thread,
  /** @export */
  emscripten_set_visibilitychange_callback_on_thread: _emscripten_set_visibilitychange_callback_on_thread,
  /** @export */
  emscripten_set_wheel_callback_on_thread: _emscripten_set_wheel_callback_on_thread,
  /** @export */
  emscripten_set_window_title: _emscripten_set_window_title,
  /** @export */
  emscripten_sleep: _emscripten_sleep,
  /** @export */
  emscripten_start_fetch: _emscripten_start_fetch,
  /** @export */
  emscripten_webgl_create_context: _emscripten_webgl_create_context,
  /** @export */
  emscripten_webgl_destroy_context: _emscripten_webgl_destroy_context,
  /** @export */
  emscripten_webgl_enable_extension: _emscripten_webgl_enable_extension,
  /** @export */
  emscripten_webgl_get_context_attributes: _emscripten_webgl_get_context_attributes,
  /** @export */
  emscripten_webgl_get_current_context: _emscripten_webgl_get_current_context,
  /** @export */
  emscripten_webgl_make_context_current: _emscripten_webgl_make_context_current,
  /** @export */
  environ_get: _environ_get,
  /** @export */
  environ_sizes_get: _environ_sizes_get,
  /** @export */
  exit: _exit,
  /** @export */
  fd_close: _fd_close,
  /** @export */
  fd_read: _fd_read,
  /** @export */
  fd_seek: _fd_seek,
  /** @export */
  fd_write: _fd_write,
  /** @export */
  glActiveTexture: _glActiveTexture,
  /** @export */
  glAttachShader: _glAttachShader,
  /** @export */
  glBindBuffer: _glBindBuffer,
  /** @export */
  glBindFramebuffer: _glBindFramebuffer,
  /** @export */
  glBindRenderbuffer: _glBindRenderbuffer,
  /** @export */
  glBindTexture: _glBindTexture,
  /** @export */
  glBlendColor: _glBlendColor,
  /** @export */
  glBlendEquationSeparate: _glBlendEquationSeparate,
  /** @export */
  glBlendFuncSeparate: _glBlendFuncSeparate,
  /** @export */
  glBufferData: _glBufferData,
  /** @export */
  glBufferSubData: _glBufferSubData,
  /** @export */
  glCheckFramebufferStatus: _glCheckFramebufferStatus,
  /** @export */
  glClear: _glClear,
  /** @export */
  glClearColor: _glClearColor,
  /** @export */
  glClearDepthf: _glClearDepthf,
  /** @export */
  glClearStencil: _glClearStencil,
  /** @export */
  glColorMask: _glColorMask,
  /** @export */
  glCompileShader: _glCompileShader,
  /** @export */
  glCompressedTexImage2D: _glCompressedTexImage2D,
  /** @export */
  glCompressedTexSubImage2D: _glCompressedTexSubImage2D,
  /** @export */
  glCreateProgram: _glCreateProgram,
  /** @export */
  glCreateShader: _glCreateShader,
  /** @export */
  glCullFace: _glCullFace,
  /** @export */
  glDeleteBuffers: _glDeleteBuffers,
  /** @export */
  glDeleteFramebuffers: _glDeleteFramebuffers,
  /** @export */
  glDeleteProgram: _glDeleteProgram,
  /** @export */
  glDeleteRenderbuffers: _glDeleteRenderbuffers,
  /** @export */
  glDeleteShader: _glDeleteShader,
  /** @export */
  glDeleteTextures: _glDeleteTextures,
  /** @export */
  glDepthFunc: _glDepthFunc,
  /** @export */
  glDepthMask: _glDepthMask,
  /** @export */
  glDetachShader: _glDetachShader,
  /** @export */
  glDisable: _glDisable,
  /** @export */
  glDisableVertexAttribArray: _glDisableVertexAttribArray,
  /** @export */
  glDrawArrays: _glDrawArrays,
  /** @export */
  glDrawElements: _glDrawElements,
  /** @export */
  glEnable: _glEnable,
  /** @export */
  glEnableVertexAttribArray: _glEnableVertexAttribArray,
  /** @export */
  glFlush: _glFlush,
  /** @export */
  glFramebufferRenderbuffer: _glFramebufferRenderbuffer,
  /** @export */
  glFramebufferTexture2D: _glFramebufferTexture2D,
  /** @export */
  glFrontFace: _glFrontFace,
  /** @export */
  glGenBuffers: _glGenBuffers,
  /** @export */
  glGenFramebuffers: _glGenFramebuffers,
  /** @export */
  glGenRenderbuffers: _glGenRenderbuffers,
  /** @export */
  glGenTextures: _glGenTextures,
  /** @export */
  glGenerateMipmap: _glGenerateMipmap,
  /** @export */
  glGetActiveAttrib: _glGetActiveAttrib,
  /** @export */
  glGetActiveUniform: _glGetActiveUniform,
  /** @export */
  glGetAttribLocation: _glGetAttribLocation,
  /** @export */
  glGetError: _glGetError,
  /** @export */
  glGetFloatv: _glGetFloatv,
  /** @export */
  glGetIntegerv: _glGetIntegerv,
  /** @export */
  glGetProgramInfoLog: _glGetProgramInfoLog,
  /** @export */
  glGetProgramiv: _glGetProgramiv,
  /** @export */
  glGetShaderInfoLog: _glGetShaderInfoLog,
  /** @export */
  glGetShaderiv: _glGetShaderiv,
  /** @export */
  glGetString: _glGetString,
  /** @export */
  glGetUniformLocation: _glGetUniformLocation,
  /** @export */
  glLinkProgram: _glLinkProgram,
  /** @export */
  glPixelStorei: _glPixelStorei,
  /** @export */
  glReadPixels: _glReadPixels,
  /** @export */
  glRenderbufferStorage: _glRenderbufferStorage,
  /** @export */
  glScissor: _glScissor,
  /** @export */
  glShaderSource: _glShaderSource,
  /** @export */
  glStencilFuncSeparate: _glStencilFuncSeparate,
  /** @export */
  glStencilOpSeparate: _glStencilOpSeparate,
  /** @export */
  glTexImage2D: _glTexImage2D,
  /** @export */
  glTexParameterf: _glTexParameterf,
  /** @export */
  glTexParameterfv: _glTexParameterfv,
  /** @export */
  glTexParameteri: _glTexParameteri,
  /** @export */
  glTexParameteriv: _glTexParameteriv,
  /** @export */
  glTexSubImage2D: _glTexSubImage2D,
  /** @export */
  glUniform1i: _glUniform1i,
  /** @export */
  glUniform1iv: _glUniform1iv,
  /** @export */
  glUniform4f: _glUniform4f,
  /** @export */
  glUniform4fv: _glUniform4fv,
  /** @export */
  glUniformMatrix3fv: _glUniformMatrix3fv,
  /** @export */
  glUniformMatrix4fv: _glUniformMatrix4fv,
  /** @export */
  glUseProgram: _glUseProgram,
  /** @export */
  glVertexAttribPointer: _glVertexAttribPointer,
  /** @export */
  glViewport: _glViewport,
  /** @export */
  invoke_diii,
  /** @export */
  invoke_fiii,
  /** @export */
  invoke_i,
  /** @export */
  invoke_ii,
  /** @export */
  invoke_iii,
  /** @export */
  invoke_iiii,
  /** @export */
  invoke_iiiii,
  /** @export */
  invoke_iiiiid,
  /** @export */
  invoke_iiiiii,
  /** @export */
  invoke_iiiiiii,
  /** @export */
  invoke_iiiiiiii,
  /** @export */
  invoke_iiiiiiiiii,
  /** @export */
  invoke_iiiiiiiiiii,
  /** @export */
  invoke_iiiiiiiiiiii,
  /** @export */
  invoke_iiiiiiiiiiiii,
  /** @export */
  invoke_iiiiij,
  /** @export */
  invoke_j,
  /** @export */
  invoke_ji,
  /** @export */
  invoke_jiiii,
  /** @export */
  invoke_v,
  /** @export */
  invoke_vi,
  /** @export */
  invoke_vii,
  /** @export */
  invoke_viif,
  /** @export */
  invoke_viii,
  /** @export */
  invoke_viiidi,
  /** @export */
  invoke_viiifi,
  /** @export */
  invoke_viiii,
  /** @export */
  invoke_viiiii,
  /** @export */
  invoke_viiiiiii,
  /** @export */
  invoke_viiiiiiiiii,
  /** @export */
  invoke_viiiiiiiiiiiiiii,
  /** @export */
  invoke_viijii,
  /** @export */
  lock_cursor_js,
  /** @export */
  random_get: _random_get,
  /** @export */
  release_cursor_js
};
var wasmExports;
createWasm();
// Imports from the Wasm binary.
var ___wasm_call_ctors = createExportWrapper('__wasm_call_ctors', 0);
var _free = createExportWrapper('free', 1);
var _malloc = createExportWrapper('malloc', 1);
var _fflush = createExportWrapper('fflush', 1);
var _main = Module['_main'] = createExportWrapper('__main_argc_argv', 2);
var _strerror = createExportWrapper('strerror', 1);
var _FMOD_JS_MixFunction = Module['_FMOD_JS_MixFunction'] = createExportWrapper('FMOD_JS_MixFunction', 1);
var _FMOD_JS_MixerSlowpathFunction = Module['_FMOD_JS_MixerSlowpathFunction'] = createExportWrapper('FMOD_JS_MixerSlowpathFunction', 0);
var _FMOD_JS_MixerFastpathFunction = Module['_FMOD_JS_MixerFastpathFunction'] = createExportWrapper('FMOD_JS_MixerFastpathFunction', 1);
var _emscripten_stack_get_end = () => (_emscripten_stack_get_end = wasmExports['emscripten_stack_get_end'])();
var _emscripten_stack_get_base = () => (_emscripten_stack_get_base = wasmExports['emscripten_stack_get_base'])();
var _emscripten_builtin_memalign = createExportWrapper('emscripten_builtin_memalign', 2);
var _setThrew = createExportWrapper('setThrew', 2);
var __emscripten_tempret_set = createExportWrapper('_emscripten_tempret_set', 1);
var _emscripten_stack_init = () => (_emscripten_stack_init = wasmExports['emscripten_stack_init'])();
var _emscripten_stack_get_free = () => (_emscripten_stack_get_free = wasmExports['emscripten_stack_get_free'])();
var __emscripten_stack_restore = (a0) => (__emscripten_stack_restore = wasmExports['_emscripten_stack_restore'])(a0);
var __emscripten_stack_alloc = (a0) => (__emscripten_stack_alloc = wasmExports['_emscripten_stack_alloc'])(a0);
var _emscripten_stack_get_current = () => (_emscripten_stack_get_current = wasmExports['emscripten_stack_get_current'])();
var ___cxa_free_exception = createExportWrapper('__cxa_free_exception', 1);
var ___cxa_decrement_exception_refcount = createExportWrapper('__cxa_decrement_exception_refcount', 1);
var ___cxa_increment_exception_refcount = createExportWrapper('__cxa_increment_exception_refcount', 1);
var ___get_exception_message = createExportWrapper('__get_exception_message', 3);
var ___cxa_can_catch = createExportWrapper('__cxa_can_catch', 3);
var ___cxa_get_exception_ptr = createExportWrapper('__cxa_get_exception_ptr', 1);

function invoke_viiii(index,a1,a2,a3,a4) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3,a4);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iii(index,a1,a2) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiii(index,a1,a2,a3,a4) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viii(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_ii(index,a1) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiii(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_vii(index,a1,a2) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7,a8,a9);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_vi(index,a1) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiii(index,a1,a2,a3,a4,a5,a6) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiifi(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiidi(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_j(index) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)();
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
    return 0n;
  }
}

function invoke_ji(index,a1) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
    return 0n;
  }
}

function invoke_v(index) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)();
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiii(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiiii(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viijii(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiij(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiid(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiiii(index,a1,a2,a3,a4,a5,a6,a7) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_jiiii(index,a1,a2,a3,a4) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
    return 0n;
  }
}

function invoke_iiiiiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_fiii(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_diii(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_i(index) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)();
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiiiiii(index,a1,a2,a3,a4,a5,a6,a7) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiiiiiiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viif(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}


// include: postamble.js
// === Auto-generated postamble setup entry stuff ===

var calledRun;

function callMain(args = []) {
  assert(runDependencies == 0, 'cannot call main when async dependencies remain! (listen on Module["onRuntimeInitialized"])');
  assert(typeof onPreRuns === 'undefined' || onPreRuns.length == 0, 'cannot call main when preRun functions remain to be called');

  var entryFunction = _main;

  args.unshift(thisProgram);

  var argc = args.length;
  var argv = stackAlloc((argc + 1) * 4);
  var argv_ptr = argv;
  args.forEach((arg) => {
    HEAPU32[((argv_ptr)>>2)] = stringToUTF8OnStack(arg);
    argv_ptr += 4;
  });
  HEAPU32[((argv_ptr)>>2)] = 0;

  try {

    var ret = entryFunction(argc, argv);

    // if we're not running an evented main loop, it's time to exit
    exitJS(ret, /* implicit = */ true);
    return ret;
  } catch (e) {
    return handleException(e);
  }
}

function stackCheckInit() {
  // This is normally called automatically during __wasm_call_ctors but need to
  // get these values before even running any of the ctors so we call it redundantly
  // here.
  _emscripten_stack_init();
  // TODO(sbc): Move writeStackCookie to native to to avoid this.
  writeStackCookie();
}

function run(args = arguments_) {

  if (runDependencies > 0) {
    dependenciesFulfilled = run;
    return;
  }

  stackCheckInit();

  preRun();

  // a preRun added a dependency, run will be called later
  if (runDependencies > 0) {
    dependenciesFulfilled = run;
    return;
  }

  function doRun() {
    // run may have just been called through dependencies being fulfilled just in this very frame,
    // or while the async setStatus time below was happening
    assert(!calledRun);
    calledRun = true;
    Module['calledRun'] = true;

    if (ABORT) return;

    initRuntime();

    preMain();

    Module['onRuntimeInitialized']?.();
    consumedModuleProp('onRuntimeInitialized');

    var noInitialRun = Module['noInitialRun'] || false;
    if (!noInitialRun) callMain(args);

    postRun();
  }

  if (Module['setStatus']) {
    Module['setStatus']('Running...');
    setTimeout(() => {
      setTimeout(() => Module['setStatus'](''), 1);
      doRun();
    }, 1);
  } else
  {
    doRun();
  }
  checkStackCookie();
}

function checkUnflushedContent() {
  // Compiler settings do not allow exiting the runtime, so flushing
  // the streams is not possible. but in ASSERTIONS mode we check
  // if there was something to flush, and if so tell the user they
  // should request that the runtime be exitable.
  // Normally we would not even include flush() at all, but in ASSERTIONS
  // builds we do so just for this check, and here we see if there is any
  // content to flush, that is, we check if there would have been
  // something a non-ASSERTIONS build would have not seen.
  // How we flush the streams depends on whether we are in SYSCALLS_REQUIRE_FILESYSTEM=0
  // mode (which has its own special function for this; otherwise, all
  // the code is inside libc)
  var oldOut = out;
  var oldErr = err;
  var has = false;
  out = err = (x) => {
    has = true;
  }
  try { // it doesn't matter if it fails
    _fflush(0);
    // also flush in the JS FS layer
    ['stdout', 'stderr'].forEach((name) => {
      var info = FS.analyzePath('/dev/' + name);
      if (!info) return;
      var stream = info.object;
      var rdev = stream.rdev;
      var tty = TTY.ttys[rdev];
      if (tty?.output?.length) {
        has = true;
      }
    });
  } catch(e) {}
  out = oldOut;
  err = oldErr;
  if (has) {
    warnOnce('stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1 (see the Emscripten FAQ), or make sure to emit a newline when you printf etc.');
  }
}

function preInit() {
  if (Module['preInit']) {
    if (typeof Module['preInit'] == 'function') Module['preInit'] = [Module['preInit']];
    while (Module['preInit'].length > 0) {
      Module['preInit'].shift()();
    }
  }
  consumedModuleProp('preInit');
}

preInit();
run();

// end include: postamble.js

