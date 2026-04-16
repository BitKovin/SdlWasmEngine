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
// include: C:\Users\bogda_\AppData\Local\Temp\tmpsqib21fx.js

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
      var PACKAGE_NAME = 'ShooterGame/emscripten/bin/main.data';
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
Module['FS_createPath']("/GameData", "animations", true, true);
Module['FS_createPath']("/GameData/animations", "npc", true, true);
Module['FS_createPath']("/GameData/animations", "player", true, true);
Module['FS_createPath']("/GameData/animations/player", "body", true, true);
Module['FS_createPath']("/GameData", "behaviourTrees", true, true);
Module['FS_createPath']("/GameData", "env", true, true);
Module['FS_createPath']("/GameData/env", "example", true, true);
Module['FS_createPath']("/GameData", "fonts", true, true);
Module['FS_createPath']("/GameData", "maps", true, true);
Module['FS_createPath']("/GameData/maps", "proto2", true, true);
Module['FS_createPath']("/GameData", "models", true, true);
Module['FS_createPath']("/GameData/models", "effects", true, true);
Module['FS_createPath']("/GameData/models", "enemies", true, true);
Module['FS_createPath']("/GameData/models/enemies", "dog", true, true);
Module['FS_createPath']("/GameData/models/enemies", "humanAxe", true, true);
Module['FS_createPath']("/GameData/models/enemies", "humanGun", true, true);
Module['FS_createPath']("/GameData/models/enemies", "zombie", true, true);
Module['FS_createPath']("/GameData/models", "engine", true, true);
Module['FS_createPath']("/GameData/models", "enviroment", true, true);
Module['FS_createPath']("/GameData/models", "npc", true, true);
Module['FS_createPath']("/GameData/models", "pickups", true, true);
Module['FS_createPath']("/GameData/models", "player", true, true);
Module['FS_createPath']("/GameData/models/player", "bike", true, true);
Module['FS_createPath']("/GameData/models/player/bike", "textures", true, true);
Module['FS_createPath']("/GameData/models/player", "body", true, true);
Module['FS_createPath']("/GameData/models/player", "weapons", true, true);
Module['FS_createPath']("/GameData/models/player/weapons", "cane", true, true);
Module['FS_createPath']("/GameData/models/player/weapons", "cannon", true, true);
Module['FS_createPath']("/GameData/models/player/weapons", "leftHand", true, true);
Module['FS_createPath']("/GameData/models/player/weapons", "mpsd", true, true);
Module['FS_createPath']("/GameData/models/player/weapons", "pistol", true, true);
Module['FS_createPath']("/GameData/models/player/weapons", "revolver", true, true);
Module['FS_createPath']("/GameData/models/player/weapons", "shotgun", true, true);
Module['FS_createPath']("/GameData/models/player/weapons", "sniper", true, true);
Module['FS_createPath']("/GameData/models/player/weapons", "stg44", true, true);
Module['FS_createPath']("/GameData/models/player/weapons", "sword", true, true);
Module['FS_createPath']("/GameData/models/player/weapons", "swords", true, true);
Module['FS_createPath']("/GameData/models/player/weapons", "tommy", true, true);
Module['FS_createPath']("/GameData/models", "tools", true, true);
Module['FS_createPath']("/GameData/models", "weapons", true, true);
Module['FS_createPath']("/GameData", "scripts", true, true);
Module['FS_createPath']("/GameData", "shaders", true, true);
Module['FS_createPath']("/GameData/shaders", "compiled", true, true);
Module['FS_createPath']("/GameData/shaders/compiled", "linux", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux", "gl", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/gl", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/gl", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/gl", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/gl", "ui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux", "spirv", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/spirv", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/spirv", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/spirv", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/linux/spirv", "ui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled", "web", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web", "gles", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/gles", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/gles", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/gles", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/gles", "ui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web", "spirv", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/spirv", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/spirv", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/spirv", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/web/spirv", "ui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled", "windows", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows", "dx11", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx11", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx11", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx11", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx11", "ui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows", "dx12", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx12", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx12", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx12", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/dx12", "ui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows", "gl", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/gl", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/gl", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/gl", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/gl", "ui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows", "spirv", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/spirv", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/spirv", "game", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/spirv", "rmlui", true, true);
Module['FS_createPath']("/GameData/shaders/compiled/windows/spirv", "ui", true, true);
Module['FS_createPath']("/GameData/shaders", "source", true, true);
Module['FS_createPath']("/GameData/shaders/source", "bsp", true, true);
Module['FS_createPath']("/GameData/shaders/source", "game", true, true);
Module['FS_createPath']("/GameData/shaders/source", "gl", true, true);
Module['FS_createPath']("/GameData/shaders/source/gl", "game", true, true);
Module['FS_createPath']("/GameData/shaders/source/gl", "ui", true, true);
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
          }          Module['removeRunDependency']('datafile_ShooterGame/emscripten/bin/main.data');

      };
      Module['addRunDependency']('datafile_ShooterGame/emscripten/bin/main.data');

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
    loadPackage({"files": [{"filename": "/GameData/animations/npc/WrithingInPain.fbx", "start": 0, "end": 1684368}, {"filename": "/GameData/animations/npc/caution_idle.glb", "start": 1684368, "end": 3548964}, {"filename": "/GameData/animations/npc/idle.glb", "start": 3548964, "end": 5361444}, {"filename": "/GameData/animations/npc/inPain.glb", "start": 5361444, "end": 7282116}, {"filename": "/GameData/animations/npc/run.glb", "start": 7282116, "end": 9110476}, {"filename": "/GameData/animations/npc/standUp.glb", "start": 9110476, "end": 11449756}, {"filename": "/GameData/animations/npc/walk.glb", "start": 11449756, "end": 12190384}, {"filename": "/GameData/animations/player/body/idle.glb", "start": 12190384, "end": 14002864}, {"filename": "/GameData/animations/player/body/run_f.glb", "start": 14002864, "end": 15831224}, {"filename": "/GameData/arms.glb", "start": 15831224, "end": 16010988}, {"filename": "/GameData/behaviourTrees/general.bt", "start": 16010988, "end": 16094630}, {"filename": "/GameData/behaviourTrees/test", "start": 16094630, "end": 16095728}, {"filename": "/GameData/behaviourTrees/test2", "start": 16095728, "end": 16101636}, {"filename": "/GameData/behaviourTrees/test3", "start": 16101636, "end": 16103545}, {"filename": "/GameData/cat.png", "start": 16103545, "end": 16352983}, {"filename": "/GameData/cube.mtl", "start": 16352983, "end": 16353036}, {"filename": "/GameData/env/example/skybox2_overcast_cube_bk.png", "start": 16353036, "end": 16846506}, {"filename": "/GameData/env/example/skybox2_overcast_cube_dn.png", "start": 16846506, "end": 17628174}, {"filename": "/GameData/env/example/skybox2_overcast_cube_ft.png", "start": 17628174, "end": 18042959}, {"filename": "/GameData/env/example/skybox2_overcast_cube_lf.png", "start": 18042959, "end": 18479570}, {"filename": "/GameData/env/example/skybox2_overcast_cube_rt.png", "start": 18479570, "end": 18942038}, {"filename": "/GameData/env/example/skybox2_overcast_cube_up.png", "start": 18942038, "end": 19264305}, {"filename": "/GameData/env/notes.txt", "start": 19264305, "end": 19264393}, {"filename": "/GameData/env/skybox1_cube_bk.png", "start": 19264393, "end": 20488634}, {"filename": "/GameData/env/skybox1_cube_dn.png", "start": 20488634, "end": 21955582}, {"filename": "/GameData/env/skybox1_cube_ft.png", "start": 21955582, "end": 23125593}, {"filename": "/GameData/env/skybox1_cube_lf.png", "start": 23125593, "end": 24310051}, {"filename": "/GameData/env/skybox1_cube_rt.png", "start": 24310051, "end": 25523018}, {"filename": "/GameData/env/skybox1_cube_up.png", "start": 25523018, "end": 26397061}, {"filename": "/GameData/env/skybox1_night_cube.png", "start": 26397061, "end": 36869121}, {"filename": "/GameData/env/skybox2_overcast_cube_bk.png", "start": 36869121, "end": 37549223}, {"filename": "/GameData/env/skybox2_overcast_cube_dn.png", "start": 37549223, "end": 38535817}, {"filename": "/GameData/env/skybox2_overcast_cube_dn_.png", "start": 38535817, "end": 39521674}, {"filename": "/GameData/env/skybox2_overcast_cube_ft.png", "start": 39521674, "end": 40127546}, {"filename": "/GameData/env/skybox2_overcast_cube_lf.png", "start": 40127546, "end": 40775385}, {"filename": "/GameData/env/skybox2_overcast_cube_rt.png", "start": 40775385, "end": 41425508}, {"filename": "/GameData/env/skybox2_overcast_cube_up.png", "start": 41425508, "end": 41920118}, {"filename": "/GameData/env/skybox2_overcast_cube_up_.png", "start": 41920118, "end": 42414661}, {"filename": "/GameData/env/skybox2_overcast_night_cube.png", "start": 42414661, "end": 46650942}, {"filename": "/GameData/env/skybox2_overcast_night_cube_bk.png", "start": 46650942, "end": 47809326}, {"filename": "/GameData/env/skybox2_overcast_night_cube_dn.png", "start": 47809326, "end": 49316803}, {"filename": "/GameData/env/skybox2_overcast_night_cube_ft.png", "start": 49316803, "end": 50390509}, {"filename": "/GameData/env/skybox2_overcast_night_cube_lf.png", "start": 50390509, "end": 51481703}, {"filename": "/GameData/env/skybox2_overcast_night_cube_rt.png", "start": 51481703, "end": 52654712}, {"filename": "/GameData/env/skybox2_overcast_night_cube_up.png", "start": 52654712, "end": 53470270}, {"filename": "/GameData/fonts/Font Awesome 6 Free-Regular-400.ttf", "start": 53470270, "end": 53546010}, {"filename": "/GameData/fonts/Kingthings_Calligraphica_2.ttf", "start": 53546010, "end": 53575814}, {"filename": "/GameData/fonts/fa-regular-400.ttf", "start": 53575814, "end": 53643878}, {"filename": "/GameData/maps/Level.bsp", "start": 53643878, "end": 53705954}, {"filename": "/GameData/maps/blank.autosave.map", "start": 53705954, "end": 53711554}, {"filename": "/GameData/maps/blank.bak", "start": 53711554, "end": 53717154}, {"filename": "/GameData/maps/blank.bsp", "start": 53717154, "end": 53821614}, {"filename": "/GameData/maps/blank.bsp.nav", "start": 53821614, "end": 53821917}, {"filename": "/GameData/maps/blank.map", "start": 53821917, "end": 53827535}, {"filename": "/GameData/maps/blank.srf", "start": 53827535, "end": 53829018}, {"filename": "/GameData/maps/l1_town.autosave.map", "start": 53829018, "end": 54022842}, {"filename": "/GameData/maps/l1_town.bak", "start": 54022842, "end": 54216260}, {"filename": "/GameData/maps/l1_town.bsp.nav", "start": 54216260, "end": 54571377}, {"filename": "/GameData/maps/l1_town.map", "start": 54571377, "end": 54765201}, {"filename": "/GameData/maps/l1_town.srf", "start": 54765201, "end": 54865876}, {"filename": "/GameData/maps/lvl1_proto.autosave.map", "start": 54865876, "end": 55204637}, {"filename": "/GameData/maps/lvl1_proto.bak", "start": 55204637, "end": 55543374}, {"filename": "/GameData/maps/lvl1_proto.bsp.nav", "start": 55543374, "end": 55814091}, {"filename": "/GameData/maps/lvl1_proto.bsp.svd", "start": 55814091, "end": 58113531}, {"filename": "/GameData/maps/lvl1_proto.map", "start": 58113531, "end": 58452292}, {"filename": "/GameData/maps/lvl1_proto.prt", "start": 58452292, "end": 58843798}, {"filename": "/GameData/maps/lvl1_proto.srf", "start": 58843798, "end": 58984075}, {"filename": "/GameData/maps/lvl1_proto.zip", "start": 58984075, "end": 60590854}, {"filename": "/GameData/maps/proto1.bak", "start": 60590854, "end": 60592368}, {"filename": "/GameData/maps/proto1.map", "start": 60592368, "end": 60593882}, {"filename": "/GameData/maps/proto2.autosave.map", "start": 60593882, "end": 60604655}, {"filename": "/GameData/maps/proto2.bak", "start": 60604655, "end": 60619174}, {"filename": "/GameData/maps/proto2.bsp", "start": 60619174, "end": 63457262}, {"filename": "/GameData/maps/proto2.map", "start": 63457262, "end": 63472050}, {"filename": "/GameData/maps/proto2.srf", "start": 63472050, "end": 63483342}, {"filename": "/GameData/maps/proto2/lm_0000.tga", "start": 63483342, "end": 63679968}, {"filename": "/GameData/maps/proto2/lm_0001.tga", "start": 63679968, "end": 63876594}, {"filename": "/GameData/maps/t.autosave.map", "start": 63876594, "end": 63877225}, {"filename": "/GameData/maps/t.bak", "start": 63877225, "end": 63877860}, {"filename": "/GameData/maps/t.map", "start": 63877860, "end": 63878491}, {"filename": "/GameData/maps/test.autosave.map", "start": 63878491, "end": 63988760}, {"filename": "/GameData/maps/test.bak", "start": 63988760, "end": 64099159}, {"filename": "/GameData/maps/test.bsp.nav", "start": 64099159, "end": 64355639}, {"filename": "/GameData/maps/test.map", "start": 64355639, "end": 64465908}, {"filename": "/GameData/maps/test.mtl", "start": 64465908, "end": 64466185}, {"filename": "/GameData/maps/test.prt", "start": 64466185, "end": 64575653}, {"filename": "/GameData/maps/test.srf", "start": 64575653, "end": 64643672}, {"filename": "/GameData/maps/test.zip", "start": 64643672, "end": 65517024}, {"filename": "/GameData/maps/test2.bsp", "start": 65517024, "end": 68630592}, {"filename": "/GameData/maps/test2.lin", "start": 68630592, "end": 68630669}, {"filename": "/GameData/maps/test2.map", "start": 68630669, "end": 69288544}, {"filename": "/GameData/maps/test2.mtl", "start": 69288544, "end": 69289239}, {"filename": "/GameData/maps/test2.srf", "start": 69289239, "end": 69963242}, {"filename": "/GameData/maps/test3.bsp", "start": 69963242, "end": 78869318}, {"filename": "/GameData/maps/test3.map", "start": 78869318, "end": 78930448}, {"filename": "/GameData/maps/test3.srf", "start": 78930448, "end": 78986590}, {"filename": "/GameData/maps/test4.bak", "start": 78986590, "end": 78991823}, {"filename": "/GameData/maps/test4.map", "start": 78991823, "end": 79001148}, {"filename": "/GameData/maps/test_interior.autosave.map", "start": 79001148, "end": 79005588}, {"filename": "/GameData/maps/test_interior.bak", "start": 79005588, "end": 79009909}, {"filename": "/GameData/maps/test_interior.bsp", "start": 79009909, "end": 79090153}, {"filename": "/GameData/maps/test_interior.bsp.nav", "start": 79090153, "end": 79092652}, {"filename": "/GameData/maps/test_interior.bsp.svd", "start": 79092652, "end": 79139980}, {"filename": "/GameData/maps/test_interior.map", "start": 79139980, "end": 79144420}, {"filename": "/GameData/maps/test_interior.srf", "start": 79144420, "end": 79146818}, {"filename": "/GameData/models/cube.mtl", "start": 79146818, "end": 79146871}, {"filename": "/GameData/models/cube.obj", "start": 79146871, "end": 79147829}, {"filename": "/GameData/models/effects/explosion.glb", "start": 79147829, "end": 79325721}, {"filename": "/GameData/models/enemies/cultist_base.blend", "start": 79325721, "end": 84466364}, {"filename": "/GameData/models/enemies/dog/dog.fbx", "start": 84466364, "end": 84802376}, {"filename": "/GameData/models/enemies/dog/dog.glb", "start": 84802376, "end": 84930988}, {"filename": "/GameData/models/enemies/dog/dog.glb.skmm", "start": 84930988, "end": 84949610}, {"filename": "/GameData/models/enemies/humanAxe/humanAxe.glb", "start": 84949610, "end": 85384518}, {"filename": "/GameData/models/enemies/humanAxe/humanAxe.glb.skmm", "start": 85384518, "end": 85395976}, {"filename": "/GameData/models/enemies/humanAxe/humanAxe1.glb", "start": 85395976, "end": 85831112}, {"filename": "/GameData/models/enemies/humanGun/humanGun.glb", "start": 85831112, "end": 89518436}, {"filename": "/GameData/models/enemies/humanGun/humanGun.glb.skmm", "start": 89518436, "end": 89530035}, {"filename": "/GameData/models/enemies/zombie/zombie.glb", "start": 89530035, "end": 90067439}, {"filename": "/GameData/models/enemies/zombie/zombie.glb.skmm", "start": 90067439, "end": 90078897}, {"filename": "/GameData/models/engine/invSphere.mtl", "start": 90078897, "end": 90078954}, {"filename": "/GameData/models/engine/invSphere.obj", "start": 90078954, "end": 90085138}, {"filename": "/GameData/models/engine/widgetPlane.glb", "start": 90085138, "end": 90086446}, {"filename": "/GameData/models/enviroment/door.glb", "start": 90086446, "end": 95407414}, {"filename": "/GameData/models/npc/base.glb", "start": 95407414, "end": 96562566}, {"filename": "/GameData/models/npc/base.glb.skmm", "start": 96562566, "end": 96574019}, {"filename": "/GameData/models/npc/guard.glb", "start": 96574019, "end": 101460463}, {"filename": "/GameData/models/npc_base.mtl", "start": 101460463, "end": 101460704}, {"filename": "/GameData/models/npc_base.obj", "start": 101460704, "end": 101463042}, {"filename": "/GameData/models/pickups/12Cal.png", "start": 101463042, "end": 101610238}, {"filename": "/GameData/models/pickups/38Cal.png", "start": 101610238, "end": 102038059}, {"filename": "/GameData/models/pickups/762Cal.png", "start": 102038059, "end": 102181431}, {"filename": "/GameData/models/pickups/cannonAmmo.obj", "start": 102181431, "end": 102182680}, {"filename": "/GameData/models/pickups/pistolAmmo.obj", "start": 102182680, "end": 102183937}, {"filename": "/GameData/models/pickups/shotgunAmmo.obj", "start": 102183937, "end": 102185187}, {"filename": "/GameData/models/player/arms.glb", "start": 102185187, "end": 102539335}, {"filename": "/GameData/models/player/arms2.glb", "start": 102539335, "end": 103144079}, {"filename": "/GameData/models/player/arms_p.glb", "start": 103144079, "end": 103585763}, {"filename": "/GameData/models/player/bike/bike.glb", "start": 103585763, "end": 109037639}, {"filename": "/GameData/models/player/bike/textures/body.png", "start": 109037639, "end": 109274320}, {"filename": "/GameData/models/player/bike/textures/front.png", "start": 109274320, "end": 109746071}, {"filename": "/GameData/models/player/bike/textures/wheels.png", "start": 109746071, "end": 110872875}, {"filename": "/GameData/models/player/body/player_body.glb", "start": 110872875, "end": 115295087}, {"filename": "/GameData/models/player/weapons/arms.glb", "start": 115295087, "end": 115649235}, {"filename": "/GameData/models/player/weapons/arms2.glb", "start": 115649235, "end": 116253979}, {"filename": "/GameData/models/player/weapons/cane/cane.glb", "start": 116253979, "end": 116637295}, {"filename": "/GameData/models/player/weapons/cane/cane.glb.skmm", "start": 116637295, "end": 116638255}, {"filename": "/GameData/models/player/weapons/cane/cane.mtl", "start": 116638255, "end": 116638494}, {"filename": "/GameData/models/player/weapons/cane/cane.obj", "start": 116638494, "end": 116666260}, {"filename": "/GameData/models/player/weapons/cane/cane.png", "start": 116666260, "end": 116756009}, {"filename": "/GameData/models/player/weapons/cannon/cannon.glb", "start": 116756009, "end": 118805881}, {"filename": "/GameData/models/player/weapons/leftHand/empty.glb", "start": 118805881, "end": 119109885}, {"filename": "/GameData/models/player/weapons/mpsd/mpsd.glb", "start": 119109885, "end": 121910521}, {"filename": "/GameData/models/player/weapons/pistol/glock.glb", "start": 121910521, "end": 122630689}, {"filename": "/GameData/models/player/weapons/pistol/muzzle_t.png", "start": 122630689, "end": 122640097}, {"filename": "/GameData/models/player/weapons/pistol/muzzle_t_em.png", "start": 122640097, "end": 122649505}, {"filename": "/GameData/models/player/weapons/pistol/pistol.glb", "start": 122649505, "end": 122920253}, {"filename": "/GameData/models/player/weapons/pistol/pistol.png", "start": 122920253, "end": 123012016}, {"filename": "/GameData/models/player/weapons/pistol/pistol_double_tp.glb", "start": 123012016, "end": 124091528}, {"filename": "/GameData/models/player/weapons/pistol/pistol_tp.glb", "start": 124091528, "end": 124847000}, {"filename": "/GameData/models/player/weapons/revolver/revolver.glb", "start": 124847000, "end": 126095648}, {"filename": "/GameData/models/player/weapons/shotgun/shotgun.glb", "start": 126095648, "end": 126761872}, {"filename": "/GameData/models/player/weapons/sniper/sniper.glb", "start": 126761872, "end": 131925916}, {"filename": "/GameData/models/player/weapons/stg44/stg44.glb", "start": 131925916, "end": 135751200}, {"filename": "/GameData/models/player/weapons/sword/sword.glb", "start": 135751200, "end": 136365360}, {"filename": "/GameData/models/player/weapons/swords/swords_tp.glb", "start": 136365360, "end": 136703652}, {"filename": "/GameData/models/player/weapons/swords/swords_tp.glb.skmm", "start": 136703652, "end": 136706593}, {"filename": "/GameData/models/player/weapons/tommy/tommy.glb", "start": 136706593, "end": 138644133}, {"filename": "/GameData/models/test.png", "start": 138644133, "end": 138644954}, {"filename": "/GameData/models/tools/door.mtl", "start": 138644954, "end": 138645193}, {"filename": "/GameData/models/tools/door.obj", "start": 138645193, "end": 138713789}, {"filename": "/GameData/models/tools/door_texture.png", "start": 138713789, "end": 143988387}, {"filename": "/GameData/models/tools/sittingPos.mtl", "start": 143988387, "end": 143988838}, {"filename": "/GameData/models/tools/sittingPos.obj", "start": 143988838, "end": 144413397}, {"filename": "/GameData/models/weapons/glock.glb", "start": 144413397, "end": 144895345}, {"filename": "/GameData/scripts/common.shader", "start": 144895345, "end": 144896158}, {"filename": "/GameData/scripts/skies.shader", "start": 144896158, "end": 144897824}, {"filename": "/GameData/shaders/ShaderCompilationAutomation.exe", "start": 144897824, "end": 145077128}, {"filename": "/GameData/shaders/compiled/linux/gl/bsp/fs_bsp.bin", "start": 145077128, "end": 145082579}, {"filename": "/GameData/shaders/compiled/linux/gl/bsp/fs_bsp_cube.bin", "start": 145082579, "end": 145083004}, {"filename": "/GameData/shaders/compiled/linux/gl/bsp/fs_bsp_empty.bin", "start": 145083004, "end": 145083084}, {"filename": "/GameData/shaders/compiled/linux/gl/bsp/vs_bsp.bin", "start": 145083084, "end": 145085379}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_bsp.bin", "start": 145085379, "end": 145090830}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_bsp_cube.bin", "start": 145090830, "end": 145091255}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_bsp_empty.bin", "start": 145091255, "end": 145091335}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_copy.bin", "start": 145091335, "end": 145091505}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_copy_depth.bin", "start": 145091505, "end": 145091674}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_customId.bin", "start": 145091674, "end": 145092457}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_default.bin", "start": 145092457, "end": 145100146}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_default_simple.bin", "start": 145100146, "end": 145103941}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_empty.bin", "start": 145103941, "end": 145104021}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_fxaa_simple.bin", "start": 145104021, "end": 145107479}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_mask.bin", "start": 145107479, "end": 145107740}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_motionBlur.bin", "start": 145107740, "end": 145109764}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_motionBlur_apply.bin", "start": 145109764, "end": 145110176}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_postprocessing.bin", "start": 145110176, "end": 145116993}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_resolve_depth_msaa.bin", "start": 145116993, "end": 145117569}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_solidRed.bin", "start": 145117569, "end": 145117649}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_ui_flatcolor.bin", "start": 145117649, "end": 145117752}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_ui_textured.bin", "start": 145117752, "end": 145118092}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_unlit.bin", "start": 145118092, "end": 145118504}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_unlit_rect.bin", "start": 145118504, "end": 145119002}, {"filename": "/GameData/shaders/compiled/linux/gl/game/fs_debuff.bin", "start": 145119002, "end": 145121479}, {"filename": "/GameData/shaders/compiled/linux/gl/game/fs_inventory_menu_sphere.bin", "start": 145121479, "end": 145121622}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_blendmask.bin", "start": 145121622, "end": 145121874}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_blur.bin", "start": 145121874, "end": 145124999}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_color.bin", "start": 145124999, "end": 145125081}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_colormatrix.bin", "start": 145125081, "end": 145125710}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_creation.bin", "start": 145125710, "end": 145125790}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_dropshadow.bin", "start": 145125790, "end": 145126096}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_gradient.bin", "start": 145126096, "end": 145130142}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_passthrough.bin", "start": 145130142, "end": 145130303}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_texture.bin", "start": 145130303, "end": 145130495}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/vs_rmlui.bin", "start": 145130495, "end": 145130899}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/vs_rmlui_passthrough.bin", "start": 145130899, "end": 145131325}, {"filename": "/GameData/shaders/compiled/linux/gl/ui/fs_progressBar.bin", "start": 145131325, "end": 145131926}, {"filename": "/GameData/shaders/compiled/linux/gl/vs_bsp.bin", "start": 145131926, "end": 145134221}, {"filename": "/GameData/shaders/compiled/linux/gl/vs_default.bin", "start": 145134221, "end": 145138887}, {"filename": "/GameData/shaders/compiled/linux/gl/vs_fullscreen.bin", "start": 145138887, "end": 145139122}, {"filename": "/GameData/shaders/compiled/linux/gl/vs_instanced_billboard.bin", "start": 145139122, "end": 145140324}, {"filename": "/GameData/shaders/compiled/linux/gl/vs_ui.bin", "start": 145140324, "end": 145140678}, {"filename": "/GameData/shaders/compiled/linux/spirv/bsp/fs_bsp.bin", "start": 145140678, "end": 145149557}, {"filename": "/GameData/shaders/compiled/linux/spirv/bsp/fs_bsp_cube.bin", "start": 145149557, "end": 145151134}, {"filename": "/GameData/shaders/compiled/linux/spirv/bsp/fs_bsp_empty.bin", "start": 145151134, "end": 145151496}, {"filename": "/GameData/shaders/compiled/linux/spirv/bsp/vs_bsp.bin", "start": 145151496, "end": 145155210}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_bsp.bin", "start": 145155210, "end": 145164089}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_bsp_cube.bin", "start": 145164089, "end": 145165666}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_bsp_empty.bin", "start": 145165666, "end": 145166028}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_copy.bin", "start": 145166028, "end": 145166874}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_copy_depth.bin", "start": 145166874, "end": 145167743}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_customId.bin", "start": 145167743, "end": 145169608}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_default.bin", "start": 145169608, "end": 145178711}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_default_simple.bin", "start": 145178711, "end": 145184804}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_empty.bin", "start": 145184804, "end": 145185166}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_fxaa_simple.bin", "start": 145185166, "end": 145188833}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_mask.bin", "start": 145188833, "end": 145189719}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_motionBlur.bin", "start": 145189719, "end": 145193937}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_motionBlur_apply.bin", "start": 145193937, "end": 145195297}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_postprocessing.bin", "start": 145195297, "end": 145204349}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_resolve_depth_msaa.bin", "start": 145204349, "end": 145205876}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_solidRed.bin", "start": 145205876, "end": 145206254}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_ui_flatcolor.bin", "start": 145206254, "end": 145206838}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_ui_textured.bin", "start": 145206838, "end": 145208054}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_unlit.bin", "start": 145208054, "end": 145209321}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_unlit_rect.bin", "start": 145209321, "end": 145210798}, {"filename": "/GameData/shaders/compiled/linux/spirv/game/fs_debuff.bin", "start": 145210798, "end": 145213719}, {"filename": "/GameData/shaders/compiled/linux/spirv/game/fs_inventory_menu_sphere.bin", "start": 145213719, "end": 145214113}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_blendmask.bin", "start": 145214113, "end": 145215153}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_blur.bin", "start": 145215153, "end": 145217781}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_color.bin", "start": 145217781, "end": 145218187}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_colormatrix.bin", "start": 145218187, "end": 145220157}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_creation.bin", "start": 145220157, "end": 145220519}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_dropshadow.bin", "start": 145220519, "end": 145221770}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_gradient.bin", "start": 145221770, "end": 145224998}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_passthrough.bin", "start": 145224998, "end": 145225753}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_texture.bin", "start": 145225753, "end": 145226616}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/vs_rmlui.bin", "start": 145226616, "end": 145227930}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/vs_rmlui_passthrough.bin", "start": 145227930, "end": 145229332}, {"filename": "/GameData/shaders/compiled/linux/spirv/ui/fs_progressBar.bin", "start": 145229332, "end": 145231241}, {"filename": "/GameData/shaders/compiled/linux/spirv/vs_bsp.bin", "start": 145231241, "end": 145234955}, {"filename": "/GameData/shaders/compiled/linux/spirv/vs_default.bin", "start": 145234955, "end": 145241666}, {"filename": "/GameData/shaders/compiled/linux/spirv/vs_fullscreen.bin", "start": 145241666, "end": 145242404}, {"filename": "/GameData/shaders/compiled/linux/spirv/vs_instanced_billboard.bin", "start": 145242404, "end": 145245240}, {"filename": "/GameData/shaders/compiled/linux/spirv/vs_ui.bin", "start": 145245240, "end": 145246499}, {"filename": "/GameData/shaders/compiled/web/gles/bsp/fs_bsp.bin", "start": 145246499, "end": 145252128}, {"filename": "/GameData/shaders/compiled/web/gles/bsp/fs_bsp_cube.bin", "start": 145252128, "end": 145252682}, {"filename": "/GameData/shaders/compiled/web/gles/bsp/fs_bsp_empty.bin", "start": 145252682, "end": 145252841}, {"filename": "/GameData/shaders/compiled/web/gles/bsp/vs_bsp.bin", "start": 145252841, "end": 145255336}, {"filename": "/GameData/shaders/compiled/web/gles/fs_bsp.bin", "start": 145255336, "end": 145260965}, {"filename": "/GameData/shaders/compiled/web/gles/fs_bsp_cube.bin", "start": 145260965, "end": 145261519}, {"filename": "/GameData/shaders/compiled/web/gles/fs_bsp_empty.bin", "start": 145261519, "end": 145261678}, {"filename": "/GameData/shaders/compiled/web/gles/fs_copy.bin", "start": 145261678, "end": 145261972}, {"filename": "/GameData/shaders/compiled/web/gles/fs_copy_depth.bin", "start": 145261972, "end": 145262230}, {"filename": "/GameData/shaders/compiled/web/gles/fs_customId.bin", "start": 145262230, "end": 145263105}, {"filename": "/GameData/shaders/compiled/web/gles/fs_default.bin", "start": 145263105, "end": 145270956}, {"filename": "/GameData/shaders/compiled/web/gles/fs_default_simple.bin", "start": 145270956, "end": 145274918}, {"filename": "/GameData/shaders/compiled/web/gles/fs_empty.bin", "start": 145274918, "end": 145275077}, {"filename": "/GameData/shaders/compiled/web/gles/fs_fxaa_simple.bin", "start": 145275077, "end": 145279024}, {"filename": "/GameData/shaders/compiled/web/gles/fs_mask.bin", "start": 145279024, "end": 145279334}, {"filename": "/GameData/shaders/compiled/web/gles/fs_motionBlur.bin", "start": 145279334, "end": 145281510}, {"filename": "/GameData/shaders/compiled/web/gles/fs_motionBlur_apply.bin", "start": 145281510, "end": 145282011}, {"filename": "/GameData/shaders/compiled/web/gles/fs_postprocessing.bin", "start": 145282011, "end": 145289346}, {"filename": "/GameData/shaders/compiled/web/gles/fs_resolve_depth_msaa.bin", "start": 145289346, "end": 145289449}, {"filename": "/GameData/shaders/compiled/web/gles/fs_solidRed.bin", "start": 145289449, "end": 145289608}, {"filename": "/GameData/shaders/compiled/web/gles/fs_ui_flatcolor.bin", "start": 145289608, "end": 145289790}, {"filename": "/GameData/shaders/compiled/web/gles/fs_ui_textured.bin", "start": 145289790, "end": 145290219}, {"filename": "/GameData/shaders/compiled/web/gles/fs_unlit.bin", "start": 145290219, "end": 145290720}, {"filename": "/GameData/shaders/compiled/web/gles/fs_unlit_rect.bin", "start": 145290720, "end": 145291307}, {"filename": "/GameData/shaders/compiled/web/gles/game/fs_debuff.bin", "start": 145291307, "end": 145293888}, {"filename": "/GameData/shaders/compiled/web/gles/game/fs_inventory_menu_sphere.bin", "start": 145293888, "end": 145294118}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_blendmask.bin", "start": 145294118, "end": 145294539}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_blur.bin", "start": 145294539, "end": 145297748}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_color.bin", "start": 145297748, "end": 145297909}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_colormatrix.bin", "start": 145297909, "end": 145298637}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_creation.bin", "start": 145298637, "end": 145298796}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_dropshadow.bin", "start": 145298796, "end": 145299226}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_gradient.bin", "start": 145299226, "end": 145303367}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_passthrough.bin", "start": 145303367, "end": 145303652}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_texture.bin", "start": 145303652, "end": 145303968}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/vs_rmlui.bin", "start": 145303968, "end": 145304464}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/vs_rmlui_passthrough.bin", "start": 145304464, "end": 145304982}, {"filename": "/GameData/shaders/compiled/web/gles/ui/fs_progressBar.bin", "start": 145304982, "end": 145305672}, {"filename": "/GameData/shaders/compiled/web/gles/vs_bsp.bin", "start": 145305672, "end": 145308167}, {"filename": "/GameData/shaders/compiled/web/gles/vs_default.bin", "start": 145308167, "end": 145313147}, {"filename": "/GameData/shaders/compiled/web/gles/vs_fullscreen.bin", "start": 145313147, "end": 145313450}, {"filename": "/GameData/shaders/compiled/web/gles/vs_instanced_billboard.bin", "start": 145313450, "end": 145314828}, {"filename": "/GameData/shaders/compiled/web/gles/vs_ui.bin", "start": 145314828, "end": 145315262}, {"filename": "/GameData/shaders/compiled/web/spirv/bsp/fs_bsp.bin", "start": 145315262, "end": 145324141}, {"filename": "/GameData/shaders/compiled/web/spirv/bsp/fs_bsp_cube.bin", "start": 145324141, "end": 145325718}, {"filename": "/GameData/shaders/compiled/web/spirv/bsp/fs_bsp_empty.bin", "start": 145325718, "end": 145326080}, {"filename": "/GameData/shaders/compiled/web/spirv/bsp/vs_bsp.bin", "start": 145326080, "end": 145329794}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_bsp.bin", "start": 145329794, "end": 145338673}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_bsp_cube.bin", "start": 145338673, "end": 145340250}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_bsp_empty.bin", "start": 145340250, "end": 145340612}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_copy.bin", "start": 145340612, "end": 145341458}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_copy_depth.bin", "start": 145341458, "end": 145342327}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_customId.bin", "start": 145342327, "end": 145344192}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_default.bin", "start": 145344192, "end": 145353295}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_default_simple.bin", "start": 145353295, "end": 145359388}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_empty.bin", "start": 145359388, "end": 145359750}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_fxaa_simple.bin", "start": 145359750, "end": 145363417}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_mask.bin", "start": 145363417, "end": 145364303}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_motionBlur.bin", "start": 145364303, "end": 145368521}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_motionBlur_apply.bin", "start": 145368521, "end": 145369881}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_postprocessing.bin", "start": 145369881, "end": 145378933}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_resolve_depth_msaa.bin", "start": 145378933, "end": 145380460}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_solidRed.bin", "start": 145380460, "end": 145380838}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_ui_flatcolor.bin", "start": 145380838, "end": 145381422}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_ui_textured.bin", "start": 145381422, "end": 145382638}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_unlit.bin", "start": 145382638, "end": 145383905}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_unlit_rect.bin", "start": 145383905, "end": 145385382}, {"filename": "/GameData/shaders/compiled/web/spirv/game/fs_debuff.bin", "start": 145385382, "end": 145388303}, {"filename": "/GameData/shaders/compiled/web/spirv/game/fs_inventory_menu_sphere.bin", "start": 145388303, "end": 145388697}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_blendmask.bin", "start": 145388697, "end": 145389737}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_blur.bin", "start": 145389737, "end": 145392365}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_color.bin", "start": 145392365, "end": 145392771}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_colormatrix.bin", "start": 145392771, "end": 145394741}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_creation.bin", "start": 145394741, "end": 145395103}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_dropshadow.bin", "start": 145395103, "end": 145396354}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_gradient.bin", "start": 145396354, "end": 145399582}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_passthrough.bin", "start": 145399582, "end": 145400337}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_texture.bin", "start": 145400337, "end": 145401200}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/vs_rmlui.bin", "start": 145401200, "end": 145402514}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/vs_rmlui_passthrough.bin", "start": 145402514, "end": 145403916}, {"filename": "/GameData/shaders/compiled/web/spirv/ui/fs_progressBar.bin", "start": 145403916, "end": 145405825}, {"filename": "/GameData/shaders/compiled/web/spirv/vs_bsp.bin", "start": 145405825, "end": 145409539}, {"filename": "/GameData/shaders/compiled/web/spirv/vs_default.bin", "start": 145409539, "end": 145416250}, {"filename": "/GameData/shaders/compiled/web/spirv/vs_fullscreen.bin", "start": 145416250, "end": 145416988}, {"filename": "/GameData/shaders/compiled/web/spirv/vs_instanced_billboard.bin", "start": 145416988, "end": 145419824}, {"filename": "/GameData/shaders/compiled/web/spirv/vs_ui.bin", "start": 145419824, "end": 145421083}, {"filename": "/GameData/shaders/compiled/windows/dx11/bsp/fs_bsp.bin", "start": 145421083, "end": 145426842}, {"filename": "/GameData/shaders/compiled/windows/dx11/bsp/fs_bsp_cube.bin", "start": 145426842, "end": 145427875}, {"filename": "/GameData/shaders/compiled/windows/dx11/bsp/fs_bsp_empty.bin", "start": 145427875, "end": 145428321}, {"filename": "/GameData/shaders/compiled/windows/dx11/bsp/vs_bsp.bin", "start": 145428321, "end": 145432283}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_bsp.bin", "start": 145432283, "end": 145438042}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_bsp_cube.bin", "start": 145438042, "end": 145439075}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_bsp_empty.bin", "start": 145439075, "end": 145439521}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_copy.bin", "start": 145439521, "end": 145440011}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_copy_depth.bin", "start": 145440011, "end": 145440512}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_customId.bin", "start": 145440512, "end": 145441749}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_default.bin", "start": 145441749, "end": 145454112}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_default_simple.bin", "start": 145454112, "end": 145458449}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_empty.bin", "start": 145458449, "end": 145458871}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_fxaa_simple.bin", "start": 145458871, "end": 145462598}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_mask.bin", "start": 145462598, "end": 145463172}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_motionBlur.bin", "start": 145463172, "end": 145466910}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_motionBlur_apply.bin", "start": 145466910, "end": 145467822}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_postprocessing.bin", "start": 145467822, "end": 145476418}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_resolve_depth_msaa.bin", "start": 145476418, "end": 145477393}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_solidRed.bin", "start": 145477393, "end": 145477843}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_ui_flatcolor.bin", "start": 145477843, "end": 145478143}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_ui_textured.bin", "start": 145478143, "end": 145478631}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_unlit.bin", "start": 145478631, "end": 145479378}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_unlit_rect.bin", "start": 145479378, "end": 145480363}, {"filename": "/GameData/shaders/compiled/windows/dx11/game/fs_debuff.bin", "start": 145480363, "end": 145484112}, {"filename": "/GameData/shaders/compiled/windows/dx11/game/fs_inventory_menu_sphere.bin", "start": 145484112, "end": 145484602}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_blendmask.bin", "start": 145484602, "end": 145485190}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_blur.bin", "start": 145485190, "end": 145487106}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_color.bin", "start": 145487106, "end": 145487432}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_colormatrix.bin", "start": 145487432, "end": 145488390}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_creation.bin", "start": 145488390, "end": 145488716}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_dropshadow.bin", "start": 145488716, "end": 145489355}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_gradient.bin", "start": 145489355, "end": 145493363}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_passthrough.bin", "start": 145493363, "end": 145493790}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_texture.bin", "start": 145493790, "end": 145494257}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/vs_rmlui.bin", "start": 145494257, "end": 145495135}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/vs_rmlui_passthrough.bin", "start": 145495135, "end": 145495925}, {"filename": "/GameData/shaders/compiled/windows/dx11/ui/fs_progressBar.bin", "start": 145495925, "end": 145496646}, {"filename": "/GameData/shaders/compiled/windows/dx11/vs_bsp.bin", "start": 145496646, "end": 145500608}, {"filename": "/GameData/shaders/compiled/windows/dx11/vs_default.bin", "start": 145500608, "end": 145510919}, {"filename": "/GameData/shaders/compiled/windows/dx11/vs_fullscreen.bin", "start": 145510919, "end": 145511357}, {"filename": "/GameData/shaders/compiled/windows/dx11/vs_instanced_billboard.bin", "start": 145511357, "end": 145513817}, {"filename": "/GameData/shaders/compiled/windows/dx11/vs_ui.bin", "start": 145513817, "end": 145514708}, {"filename": "/GameData/shaders/compiled/windows/dx12/bsp/fs_bsp.bin", "start": 145514708, "end": 145520467}, {"filename": "/GameData/shaders/compiled/windows/dx12/bsp/fs_bsp_cube.bin", "start": 145520467, "end": 145521500}, {"filename": "/GameData/shaders/compiled/windows/dx12/bsp/fs_bsp_empty.bin", "start": 145521500, "end": 145521946}, {"filename": "/GameData/shaders/compiled/windows/dx12/bsp/vs_bsp.bin", "start": 145521946, "end": 145525908}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_bsp.bin", "start": 145525908, "end": 145531667}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_bsp_cube.bin", "start": 145531667, "end": 145532700}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_bsp_empty.bin", "start": 145532700, "end": 145533146}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_copy.bin", "start": 145533146, "end": 145533636}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_copy_depth.bin", "start": 145533636, "end": 145534137}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_customId.bin", "start": 145534137, "end": 145535374}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_default.bin", "start": 145535374, "end": 145547737}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_default_simple.bin", "start": 145547737, "end": 145552074}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_empty.bin", "start": 145552074, "end": 145552496}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_fxaa_simple.bin", "start": 145552496, "end": 145556223}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_mask.bin", "start": 145556223, "end": 145556797}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_motionBlur.bin", "start": 145556797, "end": 145560535}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_motionBlur_apply.bin", "start": 145560535, "end": 145561447}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_postprocessing.bin", "start": 145561447, "end": 145570043}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_resolve_depth_msaa.bin", "start": 145570043, "end": 145571018}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_solidRed.bin", "start": 145571018, "end": 145571468}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_ui_flatcolor.bin", "start": 145571468, "end": 145571768}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_ui_textured.bin", "start": 145571768, "end": 145572256}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_unlit.bin", "start": 145572256, "end": 145573003}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_unlit_rect.bin", "start": 145573003, "end": 145573988}, {"filename": "/GameData/shaders/compiled/windows/dx12/game/fs_debuff.bin", "start": 145573988, "end": 145577737}, {"filename": "/GameData/shaders/compiled/windows/dx12/game/fs_inventory_menu_sphere.bin", "start": 145577737, "end": 145578227}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_blendmask.bin", "start": 145578227, "end": 145578815}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_blur.bin", "start": 145578815, "end": 145580731}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_color.bin", "start": 145580731, "end": 145581057}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_colormatrix.bin", "start": 145581057, "end": 145582015}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_creation.bin", "start": 145582015, "end": 145582341}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_dropshadow.bin", "start": 145582341, "end": 145582980}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_gradient.bin", "start": 145582980, "end": 145586988}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_passthrough.bin", "start": 145586988, "end": 145587415}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_texture.bin", "start": 145587415, "end": 145587882}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/vs_rmlui.bin", "start": 145587882, "end": 145588760}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/vs_rmlui_passthrough.bin", "start": 145588760, "end": 145589550}, {"filename": "/GameData/shaders/compiled/windows/dx12/ui/fs_progressBar.bin", "start": 145589550, "end": 145590271}, {"filename": "/GameData/shaders/compiled/windows/dx12/vs_bsp.bin", "start": 145590271, "end": 145594233}, {"filename": "/GameData/shaders/compiled/windows/dx12/vs_default.bin", "start": 145594233, "end": 145604544}, {"filename": "/GameData/shaders/compiled/windows/dx12/vs_fullscreen.bin", "start": 145604544, "end": 145604982}, {"filename": "/GameData/shaders/compiled/windows/dx12/vs_instanced_billboard.bin", "start": 145604982, "end": 145607442}, {"filename": "/GameData/shaders/compiled/windows/dx12/vs_ui.bin", "start": 145607442, "end": 145608333}, {"filename": "/GameData/shaders/compiled/windows/gl/bsp/fs_bsp.bin", "start": 145608333, "end": 145613784}, {"filename": "/GameData/shaders/compiled/windows/gl/bsp/fs_bsp_cube.bin", "start": 145613784, "end": 145614209}, {"filename": "/GameData/shaders/compiled/windows/gl/bsp/fs_bsp_empty.bin", "start": 145614209, "end": 145614289}, {"filename": "/GameData/shaders/compiled/windows/gl/bsp/vs_bsp.bin", "start": 145614289, "end": 145616584}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_bsp.bin", "start": 145616584, "end": 145622035}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_bsp_cube.bin", "start": 145622035, "end": 145622460}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_bsp_empty.bin", "start": 145622460, "end": 145622540}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_copy.bin", "start": 145622540, "end": 145622710}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_copy_depth.bin", "start": 145622710, "end": 145622879}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_customId.bin", "start": 145622879, "end": 145623662}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_default.bin", "start": 145623662, "end": 145631351}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_default_simple.bin", "start": 145631351, "end": 145635146}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_empty.bin", "start": 145635146, "end": 145635226}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_fxaa_simple.bin", "start": 145635226, "end": 145638684}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_mask.bin", "start": 145638684, "end": 145638945}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_motionBlur.bin", "start": 145638945, "end": 145640969}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_motionBlur_apply.bin", "start": 145640969, "end": 145641381}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_postprocessing.bin", "start": 145641381, "end": 145648198}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_resolve_depth_msaa.bin", "start": 145648198, "end": 145648774}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_solidRed.bin", "start": 145648774, "end": 145648854}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_ui_flatcolor.bin", "start": 145648854, "end": 145648957}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_ui_textured.bin", "start": 145648957, "end": 145649297}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_unlit.bin", "start": 145649297, "end": 145649709}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_unlit_rect.bin", "start": 145649709, "end": 145650207}, {"filename": "/GameData/shaders/compiled/windows/gl/game/fs_debuff.bin", "start": 145650207, "end": 145652684}, {"filename": "/GameData/shaders/compiled/windows/gl/game/fs_inventory_menu_sphere.bin", "start": 145652684, "end": 145652827}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_blendmask.bin", "start": 145652827, "end": 145653079}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_blur.bin", "start": 145653079, "end": 145656204}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_color.bin", "start": 145656204, "end": 145656286}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_colormatrix.bin", "start": 145656286, "end": 145656915}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_creation.bin", "start": 145656915, "end": 145656995}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_dropshadow.bin", "start": 145656995, "end": 145657301}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_gradient.bin", "start": 145657301, "end": 145661347}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_passthrough.bin", "start": 145661347, "end": 145661508}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_texture.bin", "start": 145661508, "end": 145661700}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/vs_rmlui.bin", "start": 145661700, "end": 145662104}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/vs_rmlui_passthrough.bin", "start": 145662104, "end": 145662530}, {"filename": "/GameData/shaders/compiled/windows/gl/ui/fs_progressBar.bin", "start": 145662530, "end": 145663131}, {"filename": "/GameData/shaders/compiled/windows/gl/vs_bsp.bin", "start": 145663131, "end": 145665426}, {"filename": "/GameData/shaders/compiled/windows/gl/vs_default.bin", "start": 145665426, "end": 145670092}, {"filename": "/GameData/shaders/compiled/windows/gl/vs_fullscreen.bin", "start": 145670092, "end": 145670327}, {"filename": "/GameData/shaders/compiled/windows/gl/vs_instanced_billboard.bin", "start": 145670327, "end": 145671529}, {"filename": "/GameData/shaders/compiled/windows/gl/vs_ui.bin", "start": 145671529, "end": 145671883}, {"filename": "/GameData/shaders/compiled/windows/spirv/bsp/fs_bsp.bin", "start": 145671883, "end": 145680762}, {"filename": "/GameData/shaders/compiled/windows/spirv/bsp/fs_bsp_cube.bin", "start": 145680762, "end": 145682339}, {"filename": "/GameData/shaders/compiled/windows/spirv/bsp/fs_bsp_empty.bin", "start": 145682339, "end": 145682701}, {"filename": "/GameData/shaders/compiled/windows/spirv/bsp/vs_bsp.bin", "start": 145682701, "end": 145686415}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_bsp.bin", "start": 145686415, "end": 145695294}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_bsp_cube.bin", "start": 145695294, "end": 145696871}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_bsp_empty.bin", "start": 145696871, "end": 145697233}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_copy.bin", "start": 145697233, "end": 145698079}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_copy_depth.bin", "start": 145698079, "end": 145698948}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_customId.bin", "start": 145698948, "end": 145700813}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_default.bin", "start": 145700813, "end": 145709916}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_default_simple.bin", "start": 145709916, "end": 145716009}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_empty.bin", "start": 145716009, "end": 145716371}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_fxaa_simple.bin", "start": 145716371, "end": 145720038}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_mask.bin", "start": 145720038, "end": 145720924}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_motionBlur.bin", "start": 145720924, "end": 145725142}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_motionBlur_apply.bin", "start": 145725142, "end": 145726502}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_postprocessing.bin", "start": 145726502, "end": 145735554}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_resolve_depth_msaa.bin", "start": 145735554, "end": 145737081}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_solidRed.bin", "start": 145737081, "end": 145737459}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_ui_flatcolor.bin", "start": 145737459, "end": 145738043}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_ui_textured.bin", "start": 145738043, "end": 145739259}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_unlit.bin", "start": 145739259, "end": 145740526}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_unlit_rect.bin", "start": 145740526, "end": 145742003}, {"filename": "/GameData/shaders/compiled/windows/spirv/game/fs_debuff.bin", "start": 145742003, "end": 145744924}, {"filename": "/GameData/shaders/compiled/windows/spirv/game/fs_inventory_menu_sphere.bin", "start": 145744924, "end": 145745318}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_blendmask.bin", "start": 145745318, "end": 145746358}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_blur.bin", "start": 145746358, "end": 145748986}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_color.bin", "start": 145748986, "end": 145749392}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_colormatrix.bin", "start": 145749392, "end": 145751362}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_creation.bin", "start": 145751362, "end": 145751724}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_dropshadow.bin", "start": 145751724, "end": 145752975}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_gradient.bin", "start": 145752975, "end": 145756203}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_passthrough.bin", "start": 145756203, "end": 145756958}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_texture.bin", "start": 145756958, "end": 145757821}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/vs_rmlui.bin", "start": 145757821, "end": 145759135}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/vs_rmlui_passthrough.bin", "start": 145759135, "end": 145760537}, {"filename": "/GameData/shaders/compiled/windows/spirv/ui/fs_progressBar.bin", "start": 145760537, "end": 145762446}, {"filename": "/GameData/shaders/compiled/windows/spirv/vs_bsp.bin", "start": 145762446, "end": 145766160}, {"filename": "/GameData/shaders/compiled/windows/spirv/vs_default.bin", "start": 145766160, "end": 145772871}, {"filename": "/GameData/shaders/compiled/windows/spirv/vs_fullscreen.bin", "start": 145772871, "end": 145773609}, {"filename": "/GameData/shaders/compiled/windows/spirv/vs_instanced_billboard.bin", "start": 145773609, "end": 145776445}, {"filename": "/GameData/shaders/compiled/windows/spirv/vs_ui.bin", "start": 145776445, "end": 145777704}, {"filename": "/GameData/shaders/source/bgfx_compute.sh", "start": 145777704, "end": 145796125}, {"filename": "/GameData/shaders/source/bgfx_shader.sh", "start": 145796125, "end": 145824084}, {"filename": "/GameData/shaders/source/bsp/bgfx_shader.sh", "start": 145824084, "end": 145852043}, {"filename": "/GameData/shaders/source/bsp/fs_bsp.sc", "start": 145852043, "end": 145856191}, {"filename": "/GameData/shaders/source/bsp/fs_bsp_cube.sc", "start": 145856191, "end": 145856633}, {"filename": "/GameData/shaders/source/bsp/fs_bsp_empty.sc", "start": 145856633, "end": 145856850}, {"filename": "/GameData/shaders/source/bsp/varying.def.sc", "start": 145856850, "end": 145857818}, {"filename": "/GameData/shaders/source/bsp/vs_bsp.sc", "start": 145857818, "end": 145859092}, {"filename": "/GameData/shaders/source/fs_copy.sc", "start": 145859092, "end": 145859346}, {"filename": "/GameData/shaders/source/fs_copy_depth.sc", "start": 145859346, "end": 145859596}, {"filename": "/GameData/shaders/source/fs_customId.sc", "start": 145859596, "end": 145860303}, {"filename": "/GameData/shaders/source/fs_default.sc", "start": 145860303, "end": 145869039}, {"filename": "/GameData/shaders/source/fs_default_simple.sc", "start": 145869039, "end": 145872610}, {"filename": "/GameData/shaders/source/fs_empty.sc", "start": 145872610, "end": 145872812}, {"filename": "/GameData/shaders/source/fs_fxaa_simple.sc", "start": 145872812, "end": 145877002}, {"filename": "/GameData/shaders/source/fs_mask.sc", "start": 145877002, "end": 145877326}, {"filename": "/GameData/shaders/source/fs_motionBlur.sc", "start": 145877326, "end": 145879871}, {"filename": "/GameData/shaders/source/fs_motionBlur_apply.sc", "start": 145879871, "end": 145880464}, {"filename": "/GameData/shaders/source/fs_postprocessing.sc", "start": 145880464, "end": 145887733}, {"filename": "/GameData/shaders/source/fs_resolve_depth_msaa.sc", "start": 145887733, "end": 145888630}, {"filename": "/GameData/shaders/source/fs_solidRed.sc", "start": 145888630, "end": 145888850}, {"filename": "/GameData/shaders/source/fs_ui_flatcolor.sc", "start": 145888850, "end": 145888974}, {"filename": "/GameData/shaders/source/fs_ui_textured.sc", "start": 145888974, "end": 145889260}, {"filename": "/GameData/shaders/source/fs_unlit.sc", "start": 145889260, "end": 145889657}, {"filename": "/GameData/shaders/source/fs_unlit_rect.sc", "start": 145889657, "end": 145890266}, {"filename": "/GameData/shaders/source/game/fs_debuff.sc", "start": 145890266, "end": 145893063}, {"filename": "/GameData/shaders/source/game/fs_inventory_menu_sphere.sc", "start": 145893063, "end": 145893674}, {"filename": "/GameData/shaders/source/gl/bsp.frag", "start": 145893674, "end": 145897077}, {"filename": "/GameData/shaders/source/gl/bsp.vert", "start": 145897077, "end": 145897910}, {"filename": "/GameData/shaders/source/gl/bsp_cube.frag", "start": 145897910, "end": 145898337}, {"filename": "/GameData/shaders/source/gl/customId.frag", "start": 145898337, "end": 145899239}, {"filename": "/GameData/shaders/source/gl/default_pixel.frag", "start": 145899239, "end": 145902777}, {"filename": "/GameData/shaders/source/gl/default_pixel_.frag", "start": 145902777, "end": 145909736}, {"filename": "/GameData/shaders/source/gl/default_pixel_shaded.frag", "start": 145909736, "end": 145920471}, {"filename": "/GameData/shaders/source/gl/default_pixel_simple.frag", "start": 145920471, "end": 145931302}, {"filename": "/GameData/shaders/source/gl/default_vertex.vert", "start": 145931302, "end": 145933849}, {"filename": "/GameData/shaders/source/gl/empty_pixel.frag", "start": 145933849, "end": 145933916}, {"filename": "/GameData/shaders/source/gl/fog_particle.frag", "start": 145933916, "end": 145935435}, {"filename": "/GameData/shaders/source/gl/fullscreen_vertex.vert", "start": 145935435, "end": 145935639}, {"filename": "/GameData/shaders/source/gl/fxaa_simple.frag", "start": 145935639, "end": 145941042}, {"filename": "/GameData/shaders/source/gl/game/debuff.frag", "start": 145941042, "end": 145943521}, {"filename": "/GameData/shaders/source/gl/game/inventory_menu_sphere.frag", "start": 145943521, "end": 145944099}, {"filename": "/GameData/shaders/source/gl/instanced_bilboard_vertex.vert", "start": 145944099, "end": 145946038}, {"filename": "/GameData/shaders/source/gl/mask_pixel.frag", "start": 145946038, "end": 145946478}, {"filename": "/GameData/shaders/source/gl/motionBlur.frag", "start": 145946478, "end": 145949646}, {"filename": "/GameData/shaders/source/gl/motionBlur_apply.frag", "start": 145949646, "end": 145950033}, {"filename": "/GameData/shaders/source/gl/postprocessing.frag", "start": 145950033, "end": 145959042}, {"filename": "/GameData/shaders/source/gl/solidRed_pixel.frag", "start": 145959042, "end": 145959211}, {"filename": "/GameData/shaders/source/gl/texture_pixel.frag", "start": 145959211, "end": 145959397}, {"filename": "/GameData/shaders/source/gl/ui.vert", "start": 145959397, "end": 145960169}, {"filename": "/GameData/shaders/source/gl/ui/progressBar.frag", "start": 145960169, "end": 145960959}, {"filename": "/GameData/shaders/source/gl/ui_flatcolor.frag", "start": 145960959, "end": 145961093}, {"filename": "/GameData/shaders/source/gl/ui_sliced.frag", "start": 145961093, "end": 145967983}, {"filename": "/GameData/shaders/source/gl/ui_textured.frag", "start": 145967983, "end": 145968304}, {"filename": "/GameData/shaders/source/gl/unlit_pixel.frag", "start": 145968304, "end": 145968746}, {"filename": "/GameData/shaders/source/rmlui/bgfx_shader.sh", "start": 145968746, "end": 145996705}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_blendmask.sc", "start": 145996705, "end": 145997335}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_blur.sc", "start": 145997335, "end": 145998973}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_color.sc", "start": 145998973, "end": 145999264}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_colormatrix.sc", "start": 145999264, "end": 146000336}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_creation.sc", "start": 146000336, "end": 146000965}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_dropshadow.sc", "start": 146000965, "end": 146001856}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_gradient.sc", "start": 146001856, "end": 146004834}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_passthrough.sc", "start": 146004834, "end": 146005203}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_texture.sc", "start": 146005203, "end": 146005631}, {"filename": "/GameData/shaders/source/rmlui/varying.def.sc", "start": 146005631, "end": 146005875}, {"filename": "/GameData/shaders/source/rmlui/vs_rmlui.sc", "start": 146005875, "end": 146006686}, {"filename": "/GameData/shaders/source/rmlui/vs_rmlui_passthrough.sc", "start": 146006686, "end": 146007548}, {"filename": "/GameData/shaders/source/ui/fs_progressBar.sc", "start": 146007548, "end": 146008282}, {"filename": "/GameData/shaders/source/varying.def.sc", "start": 146008282, "end": 146009258}, {"filename": "/GameData/shaders/source/vs_default.sc", "start": 146009258, "end": 146012756}, {"filename": "/GameData/shaders/source/vs_fullscreen.sc", "start": 146012756, "end": 146012941}, {"filename": "/GameData/shaders/source/vs_instanced_billboard.sc", "start": 146012941, "end": 146013905}, {"filename": "/GameData/shaders/source/vs_ui.sc", "start": 146013905, "end": 146014174}, {"filename": "/GameData/sounds/banks/Desktop/Dialogue_CN.bank", "start": 146014174, "end": 146066590}, {"filename": "/GameData/sounds/banks/Desktop/Dialogue_EN.bank", "start": 146066590, "end": 146127710}, {"filename": "/GameData/sounds/banks/Desktop/Dialogue_JP.bank", "start": 146127710, "end": 146194270}, {"filename": "/GameData/sounds/banks/Desktop/Master.bank", "start": 146194270, "end": 146394462}, {"filename": "/GameData/sounds/banks/Desktop/Master.strings.bank", "start": 146394462, "end": 146399142}, {"filename": "/GameData/sounds/banks/Desktop/Music.bank", "start": 146399142, "end": 148936966}, {"filename": "/GameData/sounds/banks/Desktop/SFX.bank", "start": 148936966, "end": 171512998}, {"filename": "/GameData/sounds/banks/Desktop/VO.bank", "start": 171512998, "end": 171777254}, {"filename": "/GameData/sounds/banks/Desktop/Weapons.bank", "start": 171777254, "end": 171892742}, {"filename": "/GameData/sounds/dog/death.wav", "start": 171892742, "end": 171966128, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_attack.wav", "start": 171966128, "end": 172078652, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_attack_start.wav", "start": 172078652, "end": 172275336, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_death.wav", "start": 172275336, "end": 172429536, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_hit.wav", "start": 172429536, "end": 172503228, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_stun.wav", "start": 172503228, "end": 172596678, "audio": 1}, {"filename": "/GameData/sounds/mew.wav", "start": 172596678, "end": 172670064, "audio": 1}, {"filename": "/GameData/sounds/weapons/shotgun/shotgun_fire.wav", "start": 172670064, "end": 173130948, "audio": 1}, {"filename": "/GameData/sounds/weapons/shotgun/shotgun_fire2.wav", "start": 173130948, "end": 173591832, "audio": 1}, {"filename": "/GameData/tables/items/items.csv", "start": 173591832, "end": 173593193}, {"filename": "/GameData/tables/npc/npc_example.csv", "start": 173593193, "end": 173593881}, {"filename": "/GameData/testViewmodel.glb", "start": 173593881, "end": 174465397}, {"filename": "/GameData/textures/FPSTextures/BigWall_01.png", "start": 174465397, "end": 174476824}, {"filename": "/GameData/textures/FPSTextures/BigWall_02.png", "start": 174476824, "end": 174484094}, {"filename": "/GameData/textures/FPSTextures/BigWall_03.png", "start": 174484094, "end": 174493762}, {"filename": "/GameData/textures/FPSTextures/BigWall_04.png", "start": 174493762, "end": 174508509}, {"filename": "/GameData/textures/FPSTextures/BigWall_05.png", "start": 174508509, "end": 174513892}, {"filename": "/GameData/textures/FPSTextures/Column01.png", "start": 174513892, "end": 174517562}, {"filename": "/GameData/textures/FPSTextures/Column02.png", "start": 174517562, "end": 174519703}, {"filename": "/GameData/textures/FPSTextures/Column03.png", "start": 174519703, "end": 174523105}, {"filename": "/GameData/textures/FPSTextures/Column04.png", "start": 174523105, "end": 174529303}, {"filename": "/GameData/textures/FPSTextures/Column05.png", "start": 174529303, "end": 174531787}, {"filename": "/GameData/textures/FPSTextures/Column06.png", "start": 174531787, "end": 174537571}, {"filename": "/GameData/textures/FPSTextures/Column07.png", "start": 174537571, "end": 174543547}, {"filename": "/GameData/textures/FPSTextures/Column08.png", "start": 174543547, "end": 174549718}, {"filename": "/GameData/textures/FPSTextures/Column09.png", "start": 174549718, "end": 174556246}, {"filename": "/GameData/textures/FPSTextures/Column10.png", "start": 174556246, "end": 174558018}, {"filename": "/GameData/textures/FPSTextures/Column11.png", "start": 174558018, "end": 174562690}, {"filename": "/GameData/textures/FPSTextures/Column12.png", "start": 174562690, "end": 174568506}, {"filename": "/GameData/textures/FPSTextures/Column_13.png", "start": 174568506, "end": 174570657}, {"filename": "/GameData/textures/FPSTextures/Column_14.png", "start": 174570657, "end": 174572424}, {"filename": "/GameData/textures/FPSTextures/Column_15.png", "start": 174572424, "end": 174574391}, {"filename": "/GameData/textures/FPSTextures/Column_16.png", "start": 174574391, "end": 174577072}, {"filename": "/GameData/textures/FPSTextures/Column_17.png", "start": 174577072, "end": 174579700}, {"filename": "/GameData/textures/FPSTextures/Column_18.png", "start": 174579700, "end": 174582514}, {"filename": "/GameData/textures/FPSTextures/Crate01.png", "start": 174582514, "end": 174588720}, {"filename": "/GameData/textures/FPSTextures/Crate02.png", "start": 174588720, "end": 174594788}, {"filename": "/GameData/textures/FPSTextures/Crate03.png", "start": 174594788, "end": 174597298}, {"filename": "/GameData/textures/FPSTextures/Door_01.png", "start": 174597298, "end": 174604373}, {"filename": "/GameData/textures/FPSTextures/Door_02.png", "start": 174604373, "end": 174614164}, {"filename": "/GameData/textures/FPSTextures/Door_03.png", "start": 174614164, "end": 174626340}, {"filename": "/GameData/textures/FPSTextures/Door_04.png", "start": 174626340, "end": 174634558}, {"filename": "/GameData/textures/FPSTextures/Flat01.png", "start": 174634558, "end": 174637221}, {"filename": "/GameData/textures/FPSTextures/Flat02.png", "start": 174637221, "end": 174639975}, {"filename": "/GameData/textures/FPSTextures/Flat03.png", "start": 174639975, "end": 174644550}, {"filename": "/GameData/textures/FPSTextures/Flat04.png", "start": 174644550, "end": 174648806}, {"filename": "/GameData/textures/FPSTextures/Flat05.png", "start": 174648806, "end": 174651248}, {"filename": "/GameData/textures/FPSTextures/Flat06.png", "start": 174651248, "end": 174653333}, {"filename": "/GameData/textures/FPSTextures/Flat07.png", "start": 174653333, "end": 174658755}, {"filename": "/GameData/textures/FPSTextures/Flat08.png", "start": 174658755, "end": 174664604}, {"filename": "/GameData/textures/FPSTextures/Flat09.png", "start": 174664604, "end": 174669223}, {"filename": "/GameData/textures/FPSTextures/Flat10.png", "start": 174669223, "end": 174674169}, {"filename": "/GameData/textures/FPSTextures/Flat100.png", "start": 174674169, "end": 174680081}, {"filename": "/GameData/textures/FPSTextures/Flat101.png", "start": 174680081, "end": 174682868}, {"filename": "/GameData/textures/FPSTextures/Flat102.png", "start": 174682868, "end": 174685034}, {"filename": "/GameData/textures/FPSTextures/Flat103.png", "start": 174685034, "end": 174686581}, {"filename": "/GameData/textures/FPSTextures/Flat104.png", "start": 174686581, "end": 174688583}, {"filename": "/GameData/textures/FPSTextures/Flat105.png", "start": 174688583, "end": 174690254}, {"filename": "/GameData/textures/FPSTextures/Flat106.png", "start": 174690254, "end": 174692377}, {"filename": "/GameData/textures/FPSTextures/Flat107.png", "start": 174692377, "end": 174694755}, {"filename": "/GameData/textures/FPSTextures/Flat108.png", "start": 174694755, "end": 174696592}, {"filename": "/GameData/textures/FPSTextures/Flat109.png", "start": 174696592, "end": 174701284}, {"filename": "/GameData/textures/FPSTextures/Flat11.png", "start": 174701284, "end": 174703909}, {"filename": "/GameData/textures/FPSTextures/Flat110.png", "start": 174703909, "end": 174706111}, {"filename": "/GameData/textures/FPSTextures/Flat111.png", "start": 174706111, "end": 174711444}, {"filename": "/GameData/textures/FPSTextures/Flat112.png", "start": 174711444, "end": 174713335}, {"filename": "/GameData/textures/FPSTextures/Flat113.png", "start": 174713335, "end": 174715489}, {"filename": "/GameData/textures/FPSTextures/Flat114.png", "start": 174715489, "end": 174717372}, {"filename": "/GameData/textures/FPSTextures/Flat115.png", "start": 174717372, "end": 174720138}, {"filename": "/GameData/textures/FPSTextures/Flat116.png", "start": 174720138, "end": 174725713}, {"filename": "/GameData/textures/FPSTextures/Flat117.png", "start": 174725713, "end": 174731843}, {"filename": "/GameData/textures/FPSTextures/Flat118.png", "start": 174731843, "end": 174736935}, {"filename": "/GameData/textures/FPSTextures/Flat119.png", "start": 174736935, "end": 174739562}, {"filename": "/GameData/textures/FPSTextures/Flat12.png", "start": 174739562, "end": 174742144}, {"filename": "/GameData/textures/FPSTextures/Flat120.png", "start": 174742144, "end": 174746530}, {"filename": "/GameData/textures/FPSTextures/Flat121.png", "start": 174746530, "end": 174752058}, {"filename": "/GameData/textures/FPSTextures/Flat122.png", "start": 174752058, "end": 174757468}, {"filename": "/GameData/textures/FPSTextures/Flat123.png", "start": 174757468, "end": 174763264}, {"filename": "/GameData/textures/FPSTextures/Flat124.png", "start": 174763264, "end": 174764724}, {"filename": "/GameData/textures/FPSTextures/Flat125.png", "start": 174764724, "end": 174769281}, {"filename": "/GameData/textures/FPSTextures/Flat126.png", "start": 174769281, "end": 174775483}, {"filename": "/GameData/textures/FPSTextures/Flat127.png", "start": 174775483, "end": 174781533}, {"filename": "/GameData/textures/FPSTextures/Flat128.png", "start": 174781533, "end": 174783740}, {"filename": "/GameData/textures/FPSTextures/Flat129.png", "start": 174783740, "end": 174786587}, {"filename": "/GameData/textures/FPSTextures/Flat13.png", "start": 174786587, "end": 174792233}, {"filename": "/GameData/textures/FPSTextures/Flat130.png", "start": 174792233, "end": 174798323}, {"filename": "/GameData/textures/FPSTextures/Flat131.png", "start": 174798323, "end": 174800593}, {"filename": "/GameData/textures/FPSTextures/Flat132.png", "start": 174800593, "end": 174802161}, {"filename": "/GameData/textures/FPSTextures/Flat133.png", "start": 174802161, "end": 174805806}, {"filename": "/GameData/textures/FPSTextures/Flat134.png", "start": 174805806, "end": 174815367}, {"filename": "/GameData/textures/FPSTextures/Flat135.png", "start": 174815367, "end": 174821732}, {"filename": "/GameData/textures/FPSTextures/Flat136.png", "start": 174821732, "end": 174824338}, {"filename": "/GameData/textures/FPSTextures/Flat137.png", "start": 174824338, "end": 174826842}, {"filename": "/GameData/textures/FPSTextures/Flat138.png", "start": 174826842, "end": 174831719}, {"filename": "/GameData/textures/FPSTextures/Flat139.png", "start": 174831719, "end": 174833335}, {"filename": "/GameData/textures/FPSTextures/Flat14.png", "start": 174833335, "end": 174835107}, {"filename": "/GameData/textures/FPSTextures/Flat140.png", "start": 174835107, "end": 174842613}, {"filename": "/GameData/textures/FPSTextures/Flat141.png", "start": 174842613, "end": 174845806}, {"filename": "/GameData/textures/FPSTextures/Flat142.png", "start": 174845806, "end": 174851619}, {"filename": "/GameData/textures/FPSTextures/Flat15.png", "start": 174851619, "end": 174855368}, {"filename": "/GameData/textures/FPSTextures/Flat16.png", "start": 174855368, "end": 174859313}, {"filename": "/GameData/textures/FPSTextures/Flat17.png", "start": 174859313, "end": 174861364}, {"filename": "/GameData/textures/FPSTextures/Flat18.png", "start": 174861364, "end": 174866754}, {"filename": "/GameData/textures/FPSTextures/Flat19.png", "start": 174866754, "end": 174871614}, {"filename": "/GameData/textures/FPSTextures/Flat20.png", "start": 174871614, "end": 174877986}, {"filename": "/GameData/textures/FPSTextures/Flat21.png", "start": 174877986, "end": 174883010}, {"filename": "/GameData/textures/FPSTextures/Flat22.png", "start": 174883010, "end": 174886287}, {"filename": "/GameData/textures/FPSTextures/Flat23.png", "start": 174886287, "end": 174889158}, {"filename": "/GameData/textures/FPSTextures/Flat24.png", "start": 174889158, "end": 174891656}, {"filename": "/GameData/textures/FPSTextures/Flat25.png", "start": 174891656, "end": 174894335}, {"filename": "/GameData/textures/FPSTextures/Flat26.png", "start": 174894335, "end": 174898975}, {"filename": "/GameData/textures/FPSTextures/Flat27.png", "start": 174898975, "end": 174905522}, {"filename": "/GameData/textures/FPSTextures/Flat28.png", "start": 174905522, "end": 174911301}, {"filename": "/GameData/textures/FPSTextures/Flat29.png", "start": 174911301, "end": 174917039}, {"filename": "/GameData/textures/FPSTextures/Flat30.png", "start": 174917039, "end": 174919007}, {"filename": "/GameData/textures/FPSTextures/Flat31.png", "start": 174919007, "end": 174921508}, {"filename": "/GameData/textures/FPSTextures/Flat32.png", "start": 174921508, "end": 174926993}, {"filename": "/GameData/textures/FPSTextures/Flat33.png", "start": 174926993, "end": 174929368}, {"filename": "/GameData/textures/FPSTextures/Flat34.png", "start": 174929368, "end": 174931737}, {"filename": "/GameData/textures/FPSTextures/Flat35.png", "start": 174931737, "end": 174933830}, {"filename": "/GameData/textures/FPSTextures/Flat36.png", "start": 174933830, "end": 174935720}, {"filename": "/GameData/textures/FPSTextures/Flat37.png", "start": 174935720, "end": 174941587}, {"filename": "/GameData/textures/FPSTextures/Flat38.png", "start": 174941587, "end": 174947435}, {"filename": "/GameData/textures/FPSTextures/Flat39.png", "start": 174947435, "end": 174952856}, {"filename": "/GameData/textures/FPSTextures/Flat40.png", "start": 174952856, "end": 174958582}, {"filename": "/GameData/textures/FPSTextures/Flat41.png", "start": 174958582, "end": 174960348}, {"filename": "/GameData/textures/FPSTextures/Flat42.png", "start": 174960348, "end": 174966616}, {"filename": "/GameData/textures/FPSTextures/Flat43.png", "start": 174966616, "end": 174969270}, {"filename": "/GameData/textures/FPSTextures/Flat44.png", "start": 174969270, "end": 174975221}, {"filename": "/GameData/textures/FPSTextures/Flat45.png", "start": 174975221, "end": 174977866}, {"filename": "/GameData/textures/FPSTextures/Flat46.png", "start": 174977866, "end": 174979653}, {"filename": "/GameData/textures/FPSTextures/Flat47.png", "start": 174979653, "end": 174981562}, {"filename": "/GameData/textures/FPSTextures/Flat48.png", "start": 174981562, "end": 174983979}, {"filename": "/GameData/textures/FPSTextures/Flat49.png", "start": 174983979, "end": 174986881}, {"filename": "/GameData/textures/FPSTextures/Flat50.png", "start": 174986881, "end": 174992842}, {"filename": "/GameData/textures/FPSTextures/Flat51.png", "start": 174992842, "end": 174998369}, {"filename": "/GameData/textures/FPSTextures/Flat52.png", "start": 174998369, "end": 175000305}, {"filename": "/GameData/textures/FPSTextures/Flat53.png", "start": 175000305, "end": 175005832}, {"filename": "/GameData/textures/FPSTextures/Flat54.png", "start": 175005832, "end": 175012674}, {"filename": "/GameData/textures/FPSTextures/Flat55.png", "start": 175012674, "end": 175014835}, {"filename": "/GameData/textures/FPSTextures/Flat56.png", "start": 175014835, "end": 175021502}, {"filename": "/GameData/textures/FPSTextures/Flat57.png", "start": 175021502, "end": 175027150}, {"filename": "/GameData/textures/FPSTextures/Flat58.png", "start": 175027150, "end": 175029140}, {"filename": "/GameData/textures/FPSTextures/Flat59.png", "start": 175029140, "end": 175031069}, {"filename": "/GameData/textures/FPSTextures/Flat60.png", "start": 175031069, "end": 175032878}, {"filename": "/GameData/textures/FPSTextures/Flat61.png", "start": 175032878, "end": 175035448}, {"filename": "/GameData/textures/FPSTextures/Flat62.png", "start": 175035448, "end": 175041615}, {"filename": "/GameData/textures/FPSTextures/Flat63.png", "start": 175041615, "end": 175047636}, {"filename": "/GameData/textures/FPSTextures/Flat64.png", "start": 175047636, "end": 175053371}, {"filename": "/GameData/textures/FPSTextures/Flat65.png", "start": 175053371, "end": 175056259}, {"filename": "/GameData/textures/FPSTextures/Flat66.png", "start": 175056259, "end": 175058186}, {"filename": "/GameData/textures/FPSTextures/Flat67.png", "start": 175058186, "end": 175060219}, {"filename": "/GameData/textures/FPSTextures/Flat68.png", "start": 175060219, "end": 175062381}, {"filename": "/GameData/textures/FPSTextures/Flat69.png", "start": 175062381, "end": 175067875}, {"filename": "/GameData/textures/FPSTextures/Flat70.png", "start": 175067875, "end": 175073451}, {"filename": "/GameData/textures/FPSTextures/Flat71.png", "start": 175073451, "end": 175075315}, {"filename": "/GameData/textures/FPSTextures/Flat72.png", "start": 175075315, "end": 175078360}, {"filename": "/GameData/textures/FPSTextures/Flat73.png", "start": 175078360, "end": 175081083}, {"filename": "/GameData/textures/FPSTextures/Flat74.png", "start": 175081083, "end": 175083716}, {"filename": "/GameData/textures/FPSTextures/Flat75.png", "start": 175083716, "end": 175086592}, {"filename": "/GameData/textures/FPSTextures/Flat76.png", "start": 175086592, "end": 175088637}, {"filename": "/GameData/textures/FPSTextures/Flat77.png", "start": 175088637, "end": 175090827}, {"filename": "/GameData/textures/FPSTextures/Flat78.png", "start": 175090827, "end": 175094176}, {"filename": "/GameData/textures/FPSTextures/Flat79.png", "start": 175094176, "end": 175096697}, {"filename": "/GameData/textures/FPSTextures/Flat80.png", "start": 175096697, "end": 175098518}, {"filename": "/GameData/textures/FPSTextures/Flat81.png", "start": 175098518, "end": 175101106}, {"filename": "/GameData/textures/FPSTextures/Flat82.png", "start": 175101106, "end": 175103473}, {"filename": "/GameData/textures/FPSTextures/Flat83.png", "start": 175103473, "end": 175105471}, {"filename": "/GameData/textures/FPSTextures/Flat84.png", "start": 175105471, "end": 175107687}, {"filename": "/GameData/textures/FPSTextures/Flat85.png", "start": 175107687, "end": 175109781}, {"filename": "/GameData/textures/FPSTextures/Flat86.png", "start": 175109781, "end": 175112034}, {"filename": "/GameData/textures/FPSTextures/Flat87.png", "start": 175112034, "end": 175114819}, {"filename": "/GameData/textures/FPSTextures/Flat88.png", "start": 175114819, "end": 175117752}, {"filename": "/GameData/textures/FPSTextures/Flat89.png", "start": 175117752, "end": 175120610}, {"filename": "/GameData/textures/FPSTextures/Flat90.png", "start": 175120610, "end": 175123630}, {"filename": "/GameData/textures/FPSTextures/Flat91.png", "start": 175123630, "end": 175126080}, {"filename": "/GameData/textures/FPSTextures/Flat92.png", "start": 175126080, "end": 175132513}, {"filename": "/GameData/textures/FPSTextures/Flat93.png", "start": 175132513, "end": 175138519}, {"filename": "/GameData/textures/FPSTextures/Flat94.png", "start": 175138519, "end": 175146325}, {"filename": "/GameData/textures/FPSTextures/Flat95.png", "start": 175146325, "end": 175152922}, {"filename": "/GameData/textures/FPSTextures/Flat96.png", "start": 175152922, "end": 175158724}, {"filename": "/GameData/textures/FPSTextures/Flat97.png", "start": 175158724, "end": 175164913}, {"filename": "/GameData/textures/FPSTextures/Flat98.png", "start": 175164913, "end": 175170905}, {"filename": "/GameData/textures/FPSTextures/Flat99.png", "start": 175170905, "end": 175173167}, {"filename": "/GameData/textures/FPSTextures/Flat_143.png", "start": 175173167, "end": 175175734}, {"filename": "/GameData/textures/FPSTextures/Flat_144.png", "start": 175175734, "end": 175178587}, {"filename": "/GameData/textures/FPSTextures/Flat_145.png", "start": 175178587, "end": 175181318}, {"filename": "/GameData/textures/FPSTextures/Flat_146.png", "start": 175181318, "end": 175182985}, {"filename": "/GameData/textures/FPSTextures/Flat_147.png", "start": 175182985, "end": 175185663}, {"filename": "/GameData/textures/FPSTextures/Flat_148.png", "start": 175185663, "end": 175188159}, {"filename": "/GameData/textures/FPSTextures/Flat_149.png", "start": 175188159, "end": 175190518}, {"filename": "/GameData/textures/FPSTextures/Flat_150.png", "start": 175190518, "end": 175192785}, {"filename": "/GameData/textures/FPSTextures/Flat_151.png", "start": 175192785, "end": 175195454}, {"filename": "/GameData/textures/FPSTextures/Flat_152.png", "start": 175195454, "end": 175197962}, {"filename": "/GameData/textures/FPSTextures/Flat_153.png", "start": 175197962, "end": 175200971}, {"filename": "/GameData/textures/FPSTextures/Flat_154.png", "start": 175200971, "end": 175203023}, {"filename": "/GameData/textures/FPSTextures/Flat_155.png", "start": 175203023, "end": 175205921}, {"filename": "/GameData/textures/FPSTextures/Flat_156.png", "start": 175205921, "end": 175208762}, {"filename": "/GameData/textures/FPSTextures/Flat_157.png", "start": 175208762, "end": 175211896}, {"filename": "/GameData/textures/FPSTextures/Flat_158.png", "start": 175211896, "end": 175215040}, {"filename": "/GameData/textures/FPSTextures/Flat_159.png", "start": 175215040, "end": 175217282}, {"filename": "/GameData/textures/FPSTextures/Flat_160.png", "start": 175217282, "end": 175219731}, {"filename": "/GameData/textures/FPSTextures/Flat_161.png", "start": 175219731, "end": 175223081}, {"filename": "/GameData/textures/FPSTextures/Flat_162.png", "start": 175223081, "end": 175225454}, {"filename": "/GameData/textures/FPSTextures/Flat_163.png", "start": 175225454, "end": 175228545}, {"filename": "/GameData/textures/FPSTextures/Flat_164.png", "start": 175228545, "end": 175230697}, {"filename": "/GameData/textures/FPSTextures/Flat_165.png", "start": 175230697, "end": 175232857}, {"filename": "/GameData/textures/FPSTextures/Flat_166.png", "start": 175232857, "end": 175235509}, {"filename": "/GameData/textures/FPSTextures/Flat_167.png", "start": 175235509, "end": 175237782}, {"filename": "/GameData/textures/FPSTextures/Flat_168.png", "start": 175237782, "end": 175240200}, {"filename": "/GameData/textures/FPSTextures/Flat_169.png", "start": 175240200, "end": 175242957}, {"filename": "/GameData/textures/FPSTextures/Flat_170.png", "start": 175242957, "end": 175245782}, {"filename": "/GameData/textures/FPSTextures/Flat_171.png", "start": 175245782, "end": 175248613}, {"filename": "/GameData/textures/FPSTextures/Flat_173.png", "start": 175248613, "end": 175250643}, {"filename": "/GameData/textures/FPSTextures/Flat_175.png", "start": 175250643, "end": 175252726}, {"filename": "/GameData/textures/FPSTextures/Flat_176.png", "start": 175252726, "end": 175254812}, {"filename": "/GameData/textures/FPSTextures/Flat_177.png", "start": 175254812, "end": 175256010}, {"filename": "/GameData/textures/FPSTextures/Flat_178.png", "start": 175256010, "end": 175257524}, {"filename": "/GameData/textures/FPSTextures/Flat_179.png", "start": 175257524, "end": 175260007}, {"filename": "/GameData/textures/FPSTextures/Flat_180.png", "start": 175260007, "end": 175262676}, {"filename": "/GameData/textures/FPSTextures/Flat_181.png", "start": 175262676, "end": 175264468}, {"filename": "/GameData/textures/FPSTextures/Flat_182.png", "start": 175264468, "end": 175266668}, {"filename": "/GameData/textures/FPSTextures/Flat_183.png", "start": 175266668, "end": 175269211}, {"filename": "/GameData/textures/FPSTextures/Flat_184.png", "start": 175269211, "end": 175271801}, {"filename": "/GameData/textures/FPSTextures/Flat_185.png", "start": 175271801, "end": 175274691}, {"filename": "/GameData/textures/FPSTextures/Flat_186.png", "start": 175274691, "end": 175278536}, {"filename": "/GameData/textures/FPSTextures/Flat_187.png", "start": 175278536, "end": 175280487}, {"filename": "/GameData/textures/FPSTextures/Flat_188.png", "start": 175280487, "end": 175282989}, {"filename": "/GameData/textures/FPSTextures/Flat_189.png", "start": 175282989, "end": 175285325}, {"filename": "/GameData/textures/FPSTextures/Flat_190.png", "start": 175285325, "end": 175287373}, {"filename": "/GameData/textures/FPSTextures/Flat_191.png", "start": 175287373, "end": 175290053}, {"filename": "/GameData/textures/FPSTextures/Flat_192.png", "start": 175290053, "end": 175292669}, {"filename": "/GameData/textures/FPSTextures/Flat_193.png", "start": 175292669, "end": 175294478}, {"filename": "/GameData/textures/FPSTextures/License.txt", "start": 175294478, "end": 175310209}, {"filename": "/GameData/textures/FPSTextures/Light01.png", "start": 175310209, "end": 175315269}, {"filename": "/GameData/textures/FPSTextures/Light02.png", "start": 175315269, "end": 175320616}, {"filename": "/GameData/textures/FPSTextures/Light03.png", "start": 175320616, "end": 175326520}, {"filename": "/GameData/textures/FPSTextures/Light04.png", "start": 175326520, "end": 175332338}, {"filename": "/GameData/textures/FPSTextures/Light05.png", "start": 175332338, "end": 175334636}, {"filename": "/GameData/textures/FPSTextures/Light06.png", "start": 175334636, "end": 175337634}, {"filename": "/GameData/textures/FPSTextures/Light07.png", "start": 175337634, "end": 175340614}, {"filename": "/GameData/textures/FPSTextures/Light08.png", "start": 175340614, "end": 175343682}, {"filename": "/GameData/textures/FPSTextures/Light09.png", "start": 175343682, "end": 175346428}, {"filename": "/GameData/textures/FPSTextures/Light10.png", "start": 175346428, "end": 175347774}, {"filename": "/GameData/textures/FPSTextures/Metal_01.png", "start": 175347774, "end": 175354576}, {"filename": "/GameData/textures/FPSTextures/Metal_02.png", "start": 175354576, "end": 175362497}, {"filename": "/GameData/textures/FPSTextures/Step01.png", "start": 175362497, "end": 175363690}, {"filename": "/GameData/textures/FPSTextures/Step02.png", "start": 175363690, "end": 175366431}, {"filename": "/GameData/textures/FPSTextures/Step03.png", "start": 175366431, "end": 175369191}, {"filename": "/GameData/textures/FPSTextures/Step04.png", "start": 175369191, "end": 175371952}, {"filename": "/GameData/textures/FPSTextures/Step05.png", "start": 175371952, "end": 175374701}, {"filename": "/GameData/textures/FPSTextures/Step06.png", "start": 175374701, "end": 175375736}, {"filename": "/GameData/textures/FPSTextures/Step07.png", "start": 175375736, "end": 175376953}, {"filename": "/GameData/textures/FPSTextures/Step08.png", "start": 175376953, "end": 175378091}, {"filename": "/GameData/textures/FPSTextures/Strip01.png", "start": 175378091, "end": 175381424}, {"filename": "/GameData/textures/FPSTextures/Strip02.png", "start": 175381424, "end": 175384913}, {"filename": "/GameData/textures/FPSTextures/Strip03.png", "start": 175384913, "end": 175387457}, {"filename": "/GameData/textures/FPSTextures/Strip04.png", "start": 175387457, "end": 175388783}, {"filename": "/GameData/textures/FPSTextures/Strip05.png", "start": 175388783, "end": 175392868}, {"filename": "/GameData/textures/FPSTextures/Strip06.png", "start": 175392868, "end": 175397508}, {"filename": "/GameData/textures/FPSTextures/Strip07.png", "start": 175397508, "end": 175399044}, {"filename": "/GameData/textures/FPSTextures/Wall_01.png", "start": 175399044, "end": 175403946}, {"filename": "/GameData/textures/FPSTextures/Wall_02.png", "start": 175403946, "end": 175411314}, {"filename": "/GameData/textures/FPSTextures/Wall_03.png", "start": 175411314, "end": 175415032}, {"filename": "/GameData/textures/FPSTextures/Wall_04.png", "start": 175415032, "end": 175420633}, {"filename": "/GameData/textures/FPSTextures/Wall_05.png", "start": 175420633, "end": 175424609}, {"filename": "/GameData/textures/FPSTextures/Wall_06.png", "start": 175424609, "end": 175428131}, {"filename": "/GameData/textures/FPSTextures/Wall_07.png", "start": 175428131, "end": 175433115}, {"filename": "/GameData/textures/FPSTextures/Wall_08.png", "start": 175433115, "end": 175442797}, {"filename": "/GameData/textures/FPSTextures/Wall_09.png", "start": 175442797, "end": 175448184}, {"filename": "/GameData/textures/FPSTextures/Wall_10.png", "start": 175448184, "end": 175454593}, {"filename": "/GameData/textures/FPSTextures/Wall_100.png", "start": 175454593, "end": 175457514}, {"filename": "/GameData/textures/FPSTextures/Wall_101.png", "start": 175457514, "end": 175461018}, {"filename": "/GameData/textures/FPSTextures/Wall_102.png", "start": 175461018, "end": 175465491}, {"filename": "/GameData/textures/FPSTextures/Wall_103.png", "start": 175465491, "end": 175467727}, {"filename": "/GameData/textures/FPSTextures/Wall_104.png", "start": 175467727, "end": 175469508}, {"filename": "/GameData/textures/FPSTextures/Wall_105.png", "start": 175469508, "end": 175474391}, {"filename": "/GameData/textures/FPSTextures/Wall_106.png", "start": 175474391, "end": 175477752}, {"filename": "/GameData/textures/FPSTextures/Wall_107.png", "start": 175477752, "end": 175482133}, {"filename": "/GameData/textures/FPSTextures/Wall_108.png", "start": 175482133, "end": 175487615}, {"filename": "/GameData/textures/FPSTextures/Wall_109.png", "start": 175487615, "end": 175491191}, {"filename": "/GameData/textures/FPSTextures/Wall_11.png", "start": 175491191, "end": 175494451}, {"filename": "/GameData/textures/FPSTextures/Wall_110.png", "start": 175494451, "end": 175499144}, {"filename": "/GameData/textures/FPSTextures/Wall_111.png", "start": 175499144, "end": 175509725}, {"filename": "/GameData/textures/FPSTextures/Wall_112.png", "start": 175509725, "end": 175513408}, {"filename": "/GameData/textures/FPSTextures/Wall_113.png", "start": 175513408, "end": 175516668}, {"filename": "/GameData/textures/FPSTextures/Wall_114.png", "start": 175516668, "end": 175519075}, {"filename": "/GameData/textures/FPSTextures/Wall_115.png", "start": 175519075, "end": 175521679}, {"filename": "/GameData/textures/FPSTextures/Wall_116.png", "start": 175521679, "end": 175524196}, {"filename": "/GameData/textures/FPSTextures/Wall_117.png", "start": 175524196, "end": 175530110}, {"filename": "/GameData/textures/FPSTextures/Wall_118.png", "start": 175530110, "end": 175534150}, {"filename": "/GameData/textures/FPSTextures/Wall_119.png", "start": 175534150, "end": 175544362}, {"filename": "/GameData/textures/FPSTextures/Wall_12.png", "start": 175544362, "end": 175547775}, {"filename": "/GameData/textures/FPSTextures/Wall_120.png", "start": 175547775, "end": 175550242}, {"filename": "/GameData/textures/FPSTextures/Wall_121.png", "start": 175550242, "end": 175553478}, {"filename": "/GameData/textures/FPSTextures/Wall_122.png", "start": 175553478, "end": 175557074}, {"filename": "/GameData/textures/FPSTextures/Wall_123.png", "start": 175557074, "end": 175559918}, {"filename": "/GameData/textures/FPSTextures/Wall_124.png", "start": 175559918, "end": 175562115}, {"filename": "/GameData/textures/FPSTextures/Wall_125.png", "start": 175562115, "end": 175567461}, {"filename": "/GameData/textures/FPSTextures/Wall_126.png", "start": 175567461, "end": 175570132}, {"filename": "/GameData/textures/FPSTextures/Wall_127.png", "start": 175570132, "end": 175572836}, {"filename": "/GameData/textures/FPSTextures/Wall_128.png", "start": 175572836, "end": 175575458}, {"filename": "/GameData/textures/FPSTextures/Wall_129.png", "start": 175575458, "end": 175579035}, {"filename": "/GameData/textures/FPSTextures/Wall_13.png", "start": 175579035, "end": 175582392}, {"filename": "/GameData/textures/FPSTextures/Wall_130.png", "start": 175582392, "end": 175585173}, {"filename": "/GameData/textures/FPSTextures/Wall_131.png", "start": 175585173, "end": 175586747}, {"filename": "/GameData/textures/FPSTextures/Wall_132.png", "start": 175586747, "end": 175591629}, {"filename": "/GameData/textures/FPSTextures/Wall_133.png", "start": 175591629, "end": 175596047}, {"filename": "/GameData/textures/FPSTextures/Wall_134.png", "start": 175596047, "end": 175601620}, {"filename": "/GameData/textures/FPSTextures/Wall_135.png", "start": 175601620, "end": 175605335}, {"filename": "/GameData/textures/FPSTextures/Wall_136.png", "start": 175605335, "end": 175610103}, {"filename": "/GameData/textures/FPSTextures/Wall_137.png", "start": 175610103, "end": 175614720}, {"filename": "/GameData/textures/FPSTextures/Wall_138.png", "start": 175614720, "end": 175619333}, {"filename": "/GameData/textures/FPSTextures/Wall_139.png", "start": 175619333, "end": 175623247}, {"filename": "/GameData/textures/FPSTextures/Wall_14.png", "start": 175623247, "end": 175627087}, {"filename": "/GameData/textures/FPSTextures/Wall_140.png", "start": 175627087, "end": 175631047}, {"filename": "/GameData/textures/FPSTextures/Wall_141.png", "start": 175631047, "end": 175634346}, {"filename": "/GameData/textures/FPSTextures/Wall_142.png", "start": 175634346, "end": 175637057}, {"filename": "/GameData/textures/FPSTextures/Wall_143.png", "start": 175637057, "end": 175641551}, {"filename": "/GameData/textures/FPSTextures/Wall_144.png", "start": 175641551, "end": 175643773}, {"filename": "/GameData/textures/FPSTextures/Wall_145.png", "start": 175643773, "end": 175646674}, {"filename": "/GameData/textures/FPSTextures/Wall_146.png", "start": 175646674, "end": 175649419}, {"filename": "/GameData/textures/FPSTextures/Wall_147.png", "start": 175649419, "end": 175652769}, {"filename": "/GameData/textures/FPSTextures/Wall_148.png", "start": 175652769, "end": 175656190}, {"filename": "/GameData/textures/FPSTextures/Wall_149.png", "start": 175656190, "end": 175659460}, {"filename": "/GameData/textures/FPSTextures/Wall_15.png", "start": 175659460, "end": 175664590}, {"filename": "/GameData/textures/FPSTextures/Wall_150.png", "start": 175664590, "end": 175669861}, {"filename": "/GameData/textures/FPSTextures/Wall_151.png", "start": 175669861, "end": 175674068}, {"filename": "/GameData/textures/FPSTextures/Wall_152.png", "start": 175674068, "end": 175678628}, {"filename": "/GameData/textures/FPSTextures/Wall_153.png", "start": 175678628, "end": 175682904}, {"filename": "/GameData/textures/FPSTextures/Wall_154.png", "start": 175682904, "end": 175685349}, {"filename": "/GameData/textures/FPSTextures/Wall_155.png", "start": 175685349, "end": 175687609}, {"filename": "/GameData/textures/FPSTextures/Wall_156.png", "start": 175687609, "end": 175691310}, {"filename": "/GameData/textures/FPSTextures/Wall_157.png", "start": 175691310, "end": 175694177}, {"filename": "/GameData/textures/FPSTextures/Wall_158.png", "start": 175694177, "end": 175697547}, {"filename": "/GameData/textures/FPSTextures/Wall_159.png", "start": 175697547, "end": 175699638}, {"filename": "/GameData/textures/FPSTextures/Wall_16.png", "start": 175699638, "end": 175703203}, {"filename": "/GameData/textures/FPSTextures/Wall_160.png", "start": 175703203, "end": 175705938}, {"filename": "/GameData/textures/FPSTextures/Wall_161.png", "start": 175705938, "end": 175709023}, {"filename": "/GameData/textures/FPSTextures/Wall_162.png", "start": 175709023, "end": 175713096}, {"filename": "/GameData/textures/FPSTextures/Wall_163.png", "start": 175713096, "end": 175717083}, {"filename": "/GameData/textures/FPSTextures/Wall_164.png", "start": 175717083, "end": 175719844}, {"filename": "/GameData/textures/FPSTextures/Wall_166.png", "start": 175719844, "end": 175722436}, {"filename": "/GameData/textures/FPSTextures/Wall_167.png", "start": 175722436, "end": 175726974}, {"filename": "/GameData/textures/FPSTextures/Wall_168.png", "start": 175726974, "end": 175729986}, {"filename": "/GameData/textures/FPSTextures/Wall_169.png", "start": 175729986, "end": 175732422}, {"filename": "/GameData/textures/FPSTextures/Wall_17.png", "start": 175732422, "end": 175739521}, {"filename": "/GameData/textures/FPSTextures/Wall_170.png", "start": 175739521, "end": 175742435}, {"filename": "/GameData/textures/FPSTextures/Wall_171.png", "start": 175742435, "end": 175746800}, {"filename": "/GameData/textures/FPSTextures/Wall_172.png", "start": 175746800, "end": 175749013}, {"filename": "/GameData/textures/FPSTextures/Wall_173.png", "start": 175749013, "end": 175752960}, {"filename": "/GameData/textures/FPSTextures/Wall_174.png", "start": 175752960, "end": 175756840}, {"filename": "/GameData/textures/FPSTextures/Wall_175.png", "start": 175756840, "end": 175761425}, {"filename": "/GameData/textures/FPSTextures/Wall_178.png", "start": 175761425, "end": 175772662}, {"filename": "/GameData/textures/FPSTextures/Wall_179.png", "start": 175772662, "end": 175779729}, {"filename": "/GameData/textures/FPSTextures/Wall_18.png", "start": 175779729, "end": 175786639}, {"filename": "/GameData/textures/FPSTextures/Wall_180.png", "start": 175786639, "end": 175790811}, {"filename": "/GameData/textures/FPSTextures/Wall_181.png", "start": 175790811, "end": 175795134}, {"filename": "/GameData/textures/FPSTextures/Wall_182.png", "start": 175795134, "end": 175798486}, {"filename": "/GameData/textures/FPSTextures/Wall_183.png", "start": 175798486, "end": 175801621}, {"filename": "/GameData/textures/FPSTextures/Wall_184.png", "start": 175801621, "end": 175805001}, {"filename": "/GameData/textures/FPSTextures/Wall_185.png", "start": 175805001, "end": 175807948}, {"filename": "/GameData/textures/FPSTextures/Wall_186.png", "start": 175807948, "end": 175811283}, {"filename": "/GameData/textures/FPSTextures/Wall_187.png", "start": 175811283, "end": 175814676}, {"filename": "/GameData/textures/FPSTextures/Wall_188.png", "start": 175814676, "end": 175817468}, {"filename": "/GameData/textures/FPSTextures/Wall_19.png", "start": 175817468, "end": 175825067}, {"filename": "/GameData/textures/FPSTextures/Wall_20.png", "start": 175825067, "end": 175831661}, {"filename": "/GameData/textures/FPSTextures/Wall_21.png", "start": 175831661, "end": 175836156}, {"filename": "/GameData/textures/FPSTextures/Wall_22.png", "start": 175836156, "end": 175842010}, {"filename": "/GameData/textures/FPSTextures/Wall_23.png", "start": 175842010, "end": 175845028}, {"filename": "/GameData/textures/FPSTextures/Wall_24.png", "start": 175845028, "end": 175852034}, {"filename": "/GameData/textures/FPSTextures/Wall_25.png", "start": 175852034, "end": 175856862}, {"filename": "/GameData/textures/FPSTextures/Wall_26.png", "start": 175856862, "end": 175861052}, {"filename": "/GameData/textures/FPSTextures/Wall_30.png", "start": 175861052, "end": 175864814}, {"filename": "/GameData/textures/FPSTextures/Wall_31.png", "start": 175864814, "end": 175868273}, {"filename": "/GameData/textures/FPSTextures/Wall_32.png", "start": 175868273, "end": 175871694}, {"filename": "/GameData/textures/FPSTextures/Wall_33.png", "start": 175871694, "end": 175880226}, {"filename": "/GameData/textures/FPSTextures/Wall_34.png", "start": 175880226, "end": 175885170}, {"filename": "/GameData/textures/FPSTextures/Wall_35.png", "start": 175885170, "end": 175891429}, {"filename": "/GameData/textures/FPSTextures/Wall_36.png", "start": 175891429, "end": 175896175}, {"filename": "/GameData/textures/FPSTextures/Wall_37.png", "start": 175896175, "end": 175903113}, {"filename": "/GameData/textures/FPSTextures/Wall_38.png", "start": 175903113, "end": 175911928}, {"filename": "/GameData/textures/FPSTextures/Wall_39.png", "start": 175911928, "end": 175918554}, {"filename": "/GameData/textures/FPSTextures/Wall_40.png", "start": 175918554, "end": 175920244}, {"filename": "/GameData/textures/FPSTextures/Wall_41.png", "start": 175920244, "end": 175927723}, {"filename": "/GameData/textures/FPSTextures/Wall_42.png", "start": 175927723, "end": 175935336}, {"filename": "/GameData/textures/FPSTextures/Wall_43.png", "start": 175935336, "end": 175942302}, {"filename": "/GameData/textures/FPSTextures/Wall_44.png", "start": 175942302, "end": 175945343}, {"filename": "/GameData/textures/FPSTextures/Wall_45.png", "start": 175945343, "end": 175948994}, {"filename": "/GameData/textures/FPSTextures/Wall_46.png", "start": 175948994, "end": 175957720}, {"filename": "/GameData/textures/FPSTextures/Wall_47.png", "start": 175957720, "end": 175965354}, {"filename": "/GameData/textures/FPSTextures/Wall_48.png", "start": 175965354, "end": 175968732}, {"filename": "/GameData/textures/FPSTextures/Wall_49.png", "start": 175968732, "end": 175976435}, {"filename": "/GameData/textures/FPSTextures/Wall_50.png", "start": 175976435, "end": 175979812}, {"filename": "/GameData/textures/FPSTextures/Wall_51.png", "start": 175979812, "end": 175983613}, {"filename": "/GameData/textures/FPSTextures/Wall_52.png", "start": 175983613, "end": 175991118}, {"filename": "/GameData/textures/FPSTextures/Wall_53.png", "start": 175991118, "end": 175994924}, {"filename": "/GameData/textures/FPSTextures/Wall_54.png", "start": 175994924, "end": 175997913}, {"filename": "/GameData/textures/FPSTextures/Wall_55.png", "start": 175997913, "end": 176007877}, {"filename": "/GameData/textures/FPSTextures/Wall_56.png", "start": 176007877, "end": 176015195}, {"filename": "/GameData/textures/FPSTextures/Wall_57.png", "start": 176015195, "end": 176019548}, {"filename": "/GameData/textures/FPSTextures/Wall_58.png", "start": 176019548, "end": 176024195}, {"filename": "/GameData/textures/FPSTextures/Wall_59.png", "start": 176024195, "end": 176027291}, {"filename": "/GameData/textures/FPSTextures/Wall_61.png", "start": 176027291, "end": 176030469}, {"filename": "/GameData/textures/FPSTextures/Wall_62.png", "start": 176030469, "end": 176033586}, {"filename": "/GameData/textures/FPSTextures/Wall_63.png", "start": 176033586, "end": 176037706}, {"filename": "/GameData/textures/FPSTextures/Wall_64.png", "start": 176037706, "end": 176045174}, {"filename": "/GameData/textures/FPSTextures/Wall_65.png", "start": 176045174, "end": 176053120}, {"filename": "/GameData/textures/FPSTextures/Wall_66.png", "start": 176053120, "end": 176059967}, {"filename": "/GameData/textures/FPSTextures/Wall_67.png", "start": 176059967, "end": 176066894}, {"filename": "/GameData/textures/FPSTextures/Wall_68.png", "start": 176066894, "end": 176074232}, {"filename": "/GameData/textures/FPSTextures/Wall_69.png", "start": 176074232, "end": 176080478}, {"filename": "/GameData/textures/FPSTextures/Wall_70.png", "start": 176080478, "end": 176086904}, {"filename": "/GameData/textures/FPSTextures/Wall_71.png", "start": 176086904, "end": 176093967}, {"filename": "/GameData/textures/FPSTextures/Wall_72.png", "start": 176093967, "end": 176101465}, {"filename": "/GameData/textures/FPSTextures/Wall_73.png", "start": 176101465, "end": 176106673}, {"filename": "/GameData/textures/FPSTextures/Wall_74.png", "start": 176106673, "end": 176108813}, {"filename": "/GameData/textures/FPSTextures/Wall_75.png", "start": 176108813, "end": 176112568}, {"filename": "/GameData/textures/FPSTextures/Wall_76.png", "start": 176112568, "end": 176116582}, {"filename": "/GameData/textures/FPSTextures/Wall_77.png", "start": 176116582, "end": 176125318}, {"filename": "/GameData/textures/FPSTextures/Wall_78.png", "start": 176125318, "end": 176134109}, {"filename": "/GameData/textures/FPSTextures/Wall_79.png", "start": 176134109, "end": 176136170}, {"filename": "/GameData/textures/FPSTextures/Wall_80.png", "start": 176136170, "end": 176138449}, {"filename": "/GameData/textures/FPSTextures/Wall_81.png", "start": 176138449, "end": 176140909}, {"filename": "/GameData/textures/FPSTextures/Wall_82.png", "start": 176140909, "end": 176142729}, {"filename": "/GameData/textures/FPSTextures/Wall_83.png", "start": 176142729, "end": 176151461}, {"filename": "/GameData/textures/FPSTextures/Wall_84.png", "start": 176151461, "end": 176154987}, {"filename": "/GameData/textures/FPSTextures/Wall_85.png", "start": 176154987, "end": 176156751}, {"filename": "/GameData/textures/FPSTextures/Wall_86.png", "start": 176156751, "end": 176159624}, {"filename": "/GameData/textures/FPSTextures/Wall_87.png", "start": 176159624, "end": 176166634}, {"filename": "/GameData/textures/FPSTextures/Wall_88.png", "start": 176166634, "end": 176173168}, {"filename": "/GameData/textures/FPSTextures/Wall_89.png", "start": 176173168, "end": 176176912}, {"filename": "/GameData/textures/FPSTextures/Wall_90.png", "start": 176176912, "end": 176180990}, {"filename": "/GameData/textures/FPSTextures/Wall_91.png", "start": 176180990, "end": 176184985}, {"filename": "/GameData/textures/FPSTextures/Wall_92.png", "start": 176184985, "end": 176189198}, {"filename": "/GameData/textures/FPSTextures/Wall_93.png", "start": 176189198, "end": 176193487}, {"filename": "/GameData/textures/FPSTextures/Wall_94.png", "start": 176193487, "end": 176196736}, {"filename": "/GameData/textures/FPSTextures/Wall_95.png", "start": 176196736, "end": 176201644}, {"filename": "/GameData/textures/FPSTextures/Wall_96.png", "start": 176201644, "end": 176209976}, {"filename": "/GameData/textures/FPSTextures/Wall_97.png", "start": 176209976, "end": 176218738}, {"filename": "/GameData/textures/FPSTextures/Wall_98.png", "start": 176218738, "end": 176229350}, {"filename": "/GameData/textures/FPSTextures/Wall_99.png", "start": 176229350, "end": 176236421}, {"filename": "/GameData/textures/FPSTextures/flat_172.png", "start": 176236421, "end": 176238582}, {"filename": "/GameData/textures/FPSTextures/wall_165.png", "start": 176238582, "end": 176241677}, {"filename": "/GameData/textures/FPSTextures_HD/wall_wood_wallpaper.png", "start": 176241677, "end": 176287841}, {"filename": "/GameData/textures/Ground/grass.png", "start": 176287841, "end": 176291625}, {"filename": "/GameData/textures/M_Shotgun_Base_Color.png", "start": 176291625, "end": 176666767}, {"filename": "/GameData/textures/arms.png", "start": 176666767, "end": 176702622}, {"filename": "/GameData/textures/building_1/b1_f1.png", "start": 176702622, "end": 176800646}, {"filename": "/GameData/textures/building_1/b1_f2.png", "start": 176800646, "end": 176875588}, {"filename": "/GameData/textures/building_1/b1_f3.png", "start": 176875588, "end": 176952525}, {"filename": "/GameData/textures/common/trigger.png", "start": 176952525, "end": 176964401}, {"filename": "/GameData/textures/delvenPack/dlv_door1a.png", "start": 176964401, "end": 176972185}, {"filename": "/GameData/textures/delvenPack/dlv_door1b.png", "start": 176972185, "end": 176981375}, {"filename": "/GameData/textures/delvenPack/dlv_door1c.png", "start": 176981375, "end": 176989226}, {"filename": "/GameData/textures/delvenPack/dlv_door1d.png", "start": 176989226, "end": 176998398}, {"filename": "/GameData/textures/delvenPack/dlv_door2a.png", "start": 176998398, "end": 177006122}, {"filename": "/GameData/textures/delvenPack/dlv_door2b.png", "start": 177006122, "end": 177015247}, {"filename": "/GameData/textures/delvenPack/dlv_door2c.png", "start": 177015247, "end": 177023275}, {"filename": "/GameData/textures/delvenPack/dlv_door2d.png", "start": 177023275, "end": 177032503}, {"filename": "/GameData/textures/delvenPack/dlv_door3a.png", "start": 177032503, "end": 177036651}, {"filename": "/GameData/textures/delvenPack/dlv_door3b.png", "start": 177036651, "end": 177041429}, {"filename": "/GameData/textures/delvenPack/dlv_door3c.png", "start": 177041429, "end": 177045579}, {"filename": "/GameData/textures/delvenPack/dlv_door3d.png", "start": 177045579, "end": 177050371}, {"filename": "/GameData/textures/delvenPack/dlv_door4a.png", "start": 177050371, "end": 177054492}, {"filename": "/GameData/textures/delvenPack/dlv_door4b.png", "start": 177054492, "end": 177059283}, {"filename": "/GameData/textures/delvenPack/dlv_door4c.png", "start": 177059283, "end": 177063553}, {"filename": "/GameData/textures/delvenPack/dlv_door4d.png", "start": 177063553, "end": 177068389}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1a.png", "start": 177068389, "end": 177073765}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1b.png", "start": 177073765, "end": 177079893}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1c.png", "start": 177079893, "end": 177086187}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1d.png", "start": 177086187, "end": 177089823}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1e.png", "start": 177089823, "end": 177093509}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2a.png", "start": 177093509, "end": 177098234}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2b.png", "start": 177098234, "end": 177103685}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2c.png", "start": 177103685, "end": 177109363}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2d.png", "start": 177109363, "end": 177112649}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2e.png", "start": 177112649, "end": 177116010}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3a.png", "start": 177116010, "end": 177120964}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3b.png", "start": 177120964, "end": 177126635}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3c.png", "start": 177126635, "end": 177132460}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3d.png", "start": 177132460, "end": 177135840}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3e.png", "start": 177135840, "end": 177139282}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4a.png", "start": 177139282, "end": 177148700}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4b.png", "start": 177148700, "end": 177158665}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4c.png", "start": 177158665, "end": 177168794}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4d.png", "start": 177168794, "end": 177174318}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4e.png", "start": 177174318, "end": 177179888}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5a.png", "start": 177179888, "end": 177188955}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5b.png", "start": 177188955, "end": 177198602}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5c.png", "start": 177198602, "end": 177208402}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5d.png", "start": 177208402, "end": 177213812}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5e.png", "start": 177213812, "end": 177219246}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6a.png", "start": 177219246, "end": 177226380}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6b.png", "start": 177226380, "end": 177234108}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6c.png", "start": 177234108, "end": 177242003}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6d.png", "start": 177242003, "end": 177246403}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6e.png", "start": 177246403, "end": 177250861}, {"filename": "/GameData/textures/delvenPack/dlv_ground1a.png", "start": 177250861, "end": 177258783}, {"filename": "/GameData/textures/delvenPack/dlv_ground1b.png", "start": 177258783, "end": 177267619}, {"filename": "/GameData/textures/delvenPack/dlv_ground1c.png", "start": 177267619, "end": 177277115}, {"filename": "/GameData/textures/delvenPack/dlv_ground1d.png", "start": 177277115, "end": 177287158}, {"filename": "/GameData/textures/delvenPack/dlv_ground2a.png", "start": 177287158, "end": 177297026}, {"filename": "/GameData/textures/delvenPack/dlv_ground2b.png", "start": 177297026, "end": 177305933}, {"filename": "/GameData/textures/delvenPack/dlv_ground2c.png", "start": 177305933, "end": 177315658}, {"filename": "/GameData/textures/delvenPack/dlv_ground2d.png", "start": 177315658, "end": 177326079}, {"filename": "/GameData/textures/delvenPack/dlv_ground3a.png", "start": 177326079, "end": 177336138}, {"filename": "/GameData/textures/delvenPack/dlv_ground3b.png", "start": 177336138, "end": 177345850}, {"filename": "/GameData/textures/delvenPack/dlv_ground3c.png", "start": 177345850, "end": 177355286}, {"filename": "/GameData/textures/delvenPack/dlv_ground4a.png", "start": 177355286, "end": 177365477}, {"filename": "/GameData/textures/delvenPack/dlv_ground4b.png", "start": 177365477, "end": 177375927}, {"filename": "/GameData/textures/delvenPack/dlv_ground4c.png", "start": 177375927, "end": 177385868}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal1a.png", "start": 177385868, "end": 177389896}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal1b.png", "start": 177389896, "end": 177394608}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal1c.png", "start": 177394608, "end": 177398689}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal2a.png", "start": 177398689, "end": 177402742}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal2b.png", "start": 177402742, "end": 177407501}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal2c.png", "start": 177407501, "end": 177411372}, {"filename": "/GameData/textures/delvenPack/dlv_litstone1a.png", "start": 177411372, "end": 177414750}, {"filename": "/GameData/textures/delvenPack/dlv_litstone1b.png", "start": 177414750, "end": 177418963}, {"filename": "/GameData/textures/delvenPack/dlv_litstone1c.png", "start": 177418963, "end": 177422594}, {"filename": "/GameData/textures/delvenPack/dlv_litstone2a.png", "start": 177422594, "end": 177427391}, {"filename": "/GameData/textures/delvenPack/dlv_litstone2b.png", "start": 177427391, "end": 177432917}, {"filename": "/GameData/textures/delvenPack/dlv_litstone2c.png", "start": 177432917, "end": 177437160}, {"filename": "/GameData/textures/delvenPack/dlv_metalgen1.png", "start": 177437160, "end": 177442985}, {"filename": "/GameData/textures/delvenPack/dlv_metalgen2.png", "start": 177442985, "end": 177449865}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan1a.png", "start": 177449865, "end": 177457923}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan1b.png", "start": 177457923, "end": 177465393}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan1c.png", "start": 177465393, "end": 177474595}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan2a.png", "start": 177474595, "end": 177482922}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan2b.png", "start": 177482922, "end": 177490334}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan2c.png", "start": 177490334, "end": 177499533}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan3a.png", "start": 177499533, "end": 177508140}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan3b.png", "start": 177508140, "end": 177516367}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan3c.png", "start": 177516367, "end": 177525669}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan4a.png", "start": 177525669, "end": 177534430}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan4b.png", "start": 177534430, "end": 177542755}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan4c.png", "start": 177542755, "end": 177552210}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip1a.png", "start": 177552210, "end": 177558854}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip1b.png", "start": 177558854, "end": 177565366}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip2a.png", "start": 177565366, "end": 177572499}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip2b.png", "start": 177572499, "end": 177579517}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip3a.png", "start": 177579517, "end": 177586788}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip3b.png", "start": 177586788, "end": 177593866}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip4a.png", "start": 177593866, "end": 177601434}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip4b.png", "start": 177601434, "end": 177608789}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm1a.png", "start": 177608789, "end": 177615787}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm1b.png", "start": 177615787, "end": 177622320}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm1c.png", "start": 177622320, "end": 177630508}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm2a.png", "start": 177630508, "end": 177638005}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm2b.png", "start": 177638005, "end": 177644593}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm2c.png", "start": 177644593, "end": 177652854}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm3a.png", "start": 177652854, "end": 177660130}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm3b.png", "start": 177660130, "end": 177667056}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm3c.png", "start": 177667056, "end": 177675051}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm4a.png", "start": 177675051, "end": 177681931}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm4b.png", "start": 177681931, "end": 177688639}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm4c.png", "start": 177688639, "end": 177696448}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm5.png", "start": 177696448, "end": 177700107}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm6.png", "start": 177700107, "end": 177703830}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1a.png", "start": 177703830, "end": 177710622}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1b.png", "start": 177710622, "end": 177718993}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1c.png", "start": 177718993, "end": 177726663}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1d.png", "start": 177726663, "end": 177734780}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1e.png", "start": 177734780, "end": 177742672}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2a.png", "start": 177742672, "end": 177750863}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2b.png", "start": 177750863, "end": 177760375}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2c.png", "start": 177760375, "end": 177769277}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2d.png", "start": 177769277, "end": 177778528}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2e.png", "start": 177778528, "end": 177787599}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3a.png", "start": 177787599, "end": 177795730}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3b.png", "start": 177795730, "end": 177805440}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3c.png", "start": 177805440, "end": 177814412}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3d.png", "start": 177814412, "end": 177823886}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3e.png", "start": 177823886, "end": 177833113}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr4a.png", "start": 177833113, "end": 177841619}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr4b.png", "start": 177841619, "end": 177850079}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr4c.png", "start": 177850079, "end": 177858743}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr5a.png", "start": 177858743, "end": 177865922}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr5b.png", "start": 177865922, "end": 177873050}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr5c.png", "start": 177873050, "end": 177881190}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr6a.png", "start": 177881190, "end": 177889866}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr6b.png", "start": 177889866, "end": 177898281}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr6c.png", "start": 177898281, "end": 177907014}, {"filename": "/GameData/textures/delvenPack/dlv_slategen1.png", "start": 177907014, "end": 177912975}, {"filename": "/GameData/textures/delvenPack/dlv_slategen2.png", "start": 177912975, "end": 177920534}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1a.png", "start": 177920534, "end": 177926484}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1b.png", "start": 177926484, "end": 177932300}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1c.png", "start": 177932300, "end": 177938244}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1d.png", "start": 177938244, "end": 177943890}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2a.png", "start": 177943890, "end": 177952762}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2b.png", "start": 177952762, "end": 177961436}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2c.png", "start": 177961436, "end": 177970233}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2d.png", "start": 177970233, "end": 177978529}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk3a.png", "start": 177978529, "end": 177986976}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk3b.png", "start": 177986976, "end": 177995623}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4a.png", "start": 177995623, "end": 178002242}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4b.png", "start": 178002242, "end": 178008928}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4c.png", "start": 178008928, "end": 178015545}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4d.png", "start": 178015545, "end": 178022159}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4e.png", "start": 178022159, "end": 178028743}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4f.png", "start": 178028743, "end": 178035100}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5a.png", "start": 178035100, "end": 178044577}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5b.png", "start": 178044577, "end": 178054089}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5c.png", "start": 178054089, "end": 178063587}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5d.png", "start": 178063587, "end": 178073060}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5e.png", "start": 178073060, "end": 178082451}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5f.png", "start": 178082451, "end": 178091588}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk6a.png", "start": 178091588, "end": 178100513}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk6b.png", "start": 178100513, "end": 178109738}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr1a.png", "start": 178109738, "end": 178115168}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr1b.png", "start": 178115168, "end": 178121590}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr1c.png", "start": 178121590, "end": 178128613}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr2a.png", "start": 178128613, "end": 178136501}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr2b.png", "start": 178136501, "end": 178145316}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr2c.png", "start": 178145316, "end": 178154597}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr3a.png", "start": 178154597, "end": 178160300}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr3b.png", "start": 178160300, "end": 178166778}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr3c.png", "start": 178166778, "end": 178173770}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr4a.png", "start": 178173770, "end": 178182162}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr4b.png", "start": 178182162, "end": 178191231}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr4c.png", "start": 178191231, "end": 178200331}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr5a.png", "start": 178200331, "end": 178206421}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr5b.png", "start": 178206421, "end": 178212834}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr5c.png", "start": 178212834, "end": 178219752}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr6a.png", "start": 178219752, "end": 178228297}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr6b.png", "start": 178228297, "end": 178237117}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr6c.png", "start": 178237117, "end": 178246401}, {"filename": "/GameData/textures/delvenPack/dlv_stonegen1.png", "start": 178246401, "end": 178251495}, {"filename": "/GameData/textures/delvenPack/dlv_stonegen2.png", "start": 178251495, "end": 178258925}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep1a.png", "start": 178258925, "end": 178264368}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep1b.png", "start": 178264368, "end": 178269908}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep2a.png", "start": 178269908, "end": 178277757}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep2b.png", "start": 178277757, "end": 178285507}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm1a.png", "start": 178285507, "end": 178290797}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm1b.png", "start": 178290797, "end": 178296897}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm1c.png", "start": 178296897, "end": 178302473}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm2a.png", "start": 178302473, "end": 178310219}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm2b.png", "start": 178310219, "end": 178319049}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm2c.png", "start": 178319049, "end": 178327357}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm3a.png", "start": 178327357, "end": 178332657}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm3b.png", "start": 178332657, "end": 178338730}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm4a.png", "start": 178338730, "end": 178346552}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm4b.png", "start": 178346552, "end": 178355083}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm5.png", "start": 178355083, "end": 178358240}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm6.png", "start": 178358240, "end": 178362595}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf1a.png", "start": 178362595, "end": 178368206}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf1b.png", "start": 178368206, "end": 178373823}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf2a.png", "start": 178373823, "end": 178381696}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf2b.png", "start": 178381696, "end": 178389805}, {"filename": "/GameData/textures/delvenPack/dlv_wood1a.png", "start": 178389805, "end": 178394504}, {"filename": "/GameData/textures/delvenPack/dlv_wood1b.png", "start": 178394504, "end": 178399463}, {"filename": "/GameData/textures/delvenPack/dlv_wood2a.png", "start": 178399463, "end": 178406159}, {"filename": "/GameData/textures/delvenPack/dlv_wood2b.png", "start": 178406159, "end": 178413116}, {"filename": "/GameData/textures/delvenPack/dlv_wood3a.png", "start": 178413116, "end": 178418320}, {"filename": "/GameData/textures/delvenPack/dlv_wood3b.png", "start": 178418320, "end": 178423778}, {"filename": "/GameData/textures/delvenPack/dlv_wood4a.png", "start": 178423778, "end": 178430610}, {"filename": "/GameData/textures/delvenPack/dlv_wood4b.png", "start": 178430610, "end": 178437655}, {"filename": "/GameData/textures/delvenPack/dlv_wood5a.png", "start": 178437655, "end": 178443668}, {"filename": "/GameData/textures/delvenPack/dlv_wood5b.png", "start": 178443668, "end": 178450503}, {"filename": "/GameData/textures/delvenPack/dlv_wood5c.png", "start": 178450503, "end": 178457513}, {"filename": "/GameData/textures/delvenPack/dlv_wood5d.png", "start": 178457513, "end": 178465103}, {"filename": "/GameData/textures/delvenPack/dlv_wood5e.png", "start": 178465103, "end": 178474236}, {"filename": "/GameData/textures/delvenPack/dlv_wood5f.png", "start": 178474236, "end": 178483265}, {"filename": "/GameData/textures/delvenPack/dlv_wood5g.png", "start": 178483265, "end": 178491999}, {"filename": "/GameData/textures/delvenPack/dlv_wood5h.png", "start": 178491999, "end": 178500727}, {"filename": "/GameData/textures/delvenPack/dlv_wood6a.png", "start": 178500727, "end": 178508357}, {"filename": "/GameData/textures/delvenPack/dlv_wood6b.png", "start": 178508357, "end": 178516805}, {"filename": "/GameData/textures/delvenPack/dlv_wood6c.png", "start": 178516805, "end": 178524373}, {"filename": "/GameData/textures/delvenPack/dlv_wood6d.png", "start": 178524373, "end": 178532770}, {"filename": "/GameData/textures/delvenPack/dlv_wood6e.png", "start": 178532770, "end": 178542527}, {"filename": "/GameData/textures/delvenPack/dlv_wood6f.png", "start": 178542527, "end": 178552473}, {"filename": "/GameData/textures/delvenPack/dlv_wood6g.png", "start": 178552473, "end": 178561676}, {"filename": "/GameData/textures/delvenPack/dlv_wood6h.png", "start": 178561676, "end": 178571115}, {"filename": "/GameData/textures/delvenPack/dlv_woodgen1.png", "start": 178571115, "end": 178575662}, {"filename": "/GameData/textures/delvenPack/dlv_woodgen2.png", "start": 178575662, "end": 178582355}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1a.png", "start": 178582355, "end": 178590142}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1b.png", "start": 178590142, "end": 178597289}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1c.png", "start": 178597289, "end": 178601559}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1d.png", "start": 178601559, "end": 178605569}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2a.png", "start": 178605569, "end": 178612708}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2b.png", "start": 178612708, "end": 178619267}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2c.png", "start": 178619267, "end": 178623088}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2d.png", "start": 178623088, "end": 178626726}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3a.png", "start": 178626726, "end": 178633855}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3b.png", "start": 178633855, "end": 178640441}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3c.png", "start": 178640441, "end": 178644414}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3d.png", "start": 178644414, "end": 178648186}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4a.png", "start": 178648186, "end": 178659068}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4b.png", "start": 178659068, "end": 178667906}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4c.png", "start": 178667906, "end": 178673408}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4d.png", "start": 178673408, "end": 178678118}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5a.png", "start": 178678118, "end": 178688702}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5b.png", "start": 178688702, "end": 178697328}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5c.png", "start": 178697328, "end": 178702637}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5d.png", "start": 178702637, "end": 178707187}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6a.png", "start": 178707187, "end": 178716036}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6b.png", "start": 178716036, "end": 178723314}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6c.png", "start": 178723314, "end": 178727932}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6d.png", "start": 178727932, "end": 178731944}, {"filename": "/GameData/textures/generic/__TB_empty.png", "start": 178731944, "end": 178732702}, {"filename": "/GameData/textures/generic/brick.png", "start": 178732702, "end": 179245616}, {"filename": "/GameData/textures/generic/brickPBR.png", "start": 179245616, "end": 180175748}, {"filename": "/GameData/textures/generic/brickPBR_orm.png", "start": 180175748, "end": 180600599}, {"filename": "/GameData/textures/generic/bricks.png", "start": 180600599, "end": 180611519}, {"filename": "/GameData/textures/generic/cat.png", "start": 180611519, "end": 180860957}, {"filename": "/GameData/textures/generic/foil.png", "start": 180860957, "end": 181118175}, {"filename": "/GameData/textures/generic/grass.png", "start": 181118175, "end": 181246300}, {"filename": "/GameData/textures/generic/hole_t.png", "start": 181246300, "end": 181248729}, {"filename": "/GameData/textures/generic/light.png", "start": 181248729, "end": 181248857}, {"filename": "/GameData/textures/generic/light_em.png", "start": 181248857, "end": 181248985}, {"filename": "/GameData/textures/generic/mask_test_m.png", "start": 181248985, "end": 181253047}, {"filename": "/GameData/textures/generic/mirror.png", "start": 181253047, "end": 181253167}, {"filename": "/GameData/textures/generic/mirror_orm.png", "start": 181253167, "end": 181253287}, {"filename": "/GameData/textures/generic/null_m.png", "start": 181253287, "end": 181255084}, {"filename": "/GameData/textures/generic/trigger_t.png", "start": 181255084, "end": 181266960}, {"filename": "/GameData/textures/generic/white.png", "start": 181266960, "end": 181267080}, {"filename": "/GameData/textures/gloves.png", "start": 181267080, "end": 181376755}, {"filename": "/GameData/textures/jacket.png", "start": 181376755, "end": 181540567}, {"filename": "/GameData/textures/lq_conc/conc1_1.png", "start": 181540567, "end": 181553680}, {"filename": "/GameData/textures/lq_conc/conc1_10.png", "start": 181553680, "end": 181567794}, {"filename": "/GameData/textures/lq_conc/conc1_2.png", "start": 181567794, "end": 181581835}, {"filename": "/GameData/textures/lq_conc/conc1_3.png", "start": 181581835, "end": 181596345}, {"filename": "/GameData/textures/lq_conc/conc1_4.png", "start": 181596345, "end": 181610731}, {"filename": "/GameData/textures/lq_conc/conc1_5.png", "start": 181610731, "end": 181625180}, {"filename": "/GameData/textures/lq_conc/conc1_6.png", "start": 181625180, "end": 181639797}, {"filename": "/GameData/textures/lq_conc/conc1_7.png", "start": 181639797, "end": 181654394}, {"filename": "/GameData/textures/lq_conc/conc1_8.png", "start": 181654394, "end": 181668742}, {"filename": "/GameData/textures/lq_conc/conc1_9.png", "start": 181668742, "end": 181682900}, {"filename": "/GameData/textures/lq_conc/conc1_a1.png", "start": 181682900, "end": 181697348}, {"filename": "/GameData/textures/lq_conc/conc2_1.png", "start": 181697348, "end": 181706776}, {"filename": "/GameData/textures/lq_conc/conc2_10.png", "start": 181706776, "end": 181716225}, {"filename": "/GameData/textures/lq_conc/conc2_2.png", "start": 181716225, "end": 181725619}, {"filename": "/GameData/textures/lq_conc/conc2_3.png", "start": 181725619, "end": 181735264}, {"filename": "/GameData/textures/lq_conc/conc2_4.png", "start": 181735264, "end": 181744771}, {"filename": "/GameData/textures/lq_conc/conc2_5.png", "start": 181744771, "end": 181754276}, {"filename": "/GameData/textures/lq_conc/conc2_6.png", "start": 181754276, "end": 181763789}, {"filename": "/GameData/textures/lq_conc/conc2_7.png", "start": 181763789, "end": 181773334}, {"filename": "/GameData/textures/lq_conc/conc2_8.png", "start": 181773334, "end": 181782744}, {"filename": "/GameData/textures/lq_conc/conc2_9.png", "start": 181782744, "end": 181792281}, {"filename": "/GameData/textures/lq_conc/conc2_a1.png", "start": 181792281, "end": 181801311}, {"filename": "/GameData/textures/lq_conc/conc3_1.png", "start": 181801311, "end": 181809175}, {"filename": "/GameData/textures/lq_conc/conc3_10.png", "start": 181809175, "end": 181818111}, {"filename": "/GameData/textures/lq_conc/conc3_2.png", "start": 181818111, "end": 181826505}, {"filename": "/GameData/textures/lq_conc/conc3_3.png", "start": 181826505, "end": 181836732}, {"filename": "/GameData/textures/lq_conc/conc3_4.png", "start": 181836732, "end": 181846011}, {"filename": "/GameData/textures/lq_conc/conc3_5.png", "start": 181846011, "end": 181855741}, {"filename": "/GameData/textures/lq_conc/conc3_6.png", "start": 181855741, "end": 181865301}, {"filename": "/GameData/textures/lq_conc/conc3_7.png", "start": 181865301, "end": 181874126}, {"filename": "/GameData/textures/lq_conc/conc3_8.png", "start": 181874126, "end": 181882907}, {"filename": "/GameData/textures/lq_conc/conc3_9.png", "start": 181882907, "end": 181892383}, {"filename": "/GameData/textures/lq_conc/conc3_a1.png", "start": 181892383, "end": 181900790}, {"filename": "/GameData/textures/lq_conc/conc4_1.png", "start": 181900790, "end": 181906949}, {"filename": "/GameData/textures/lq_conc/conc4_10.png", "start": 181906949, "end": 181913309}, {"filename": "/GameData/textures/lq_conc/conc4_2.png", "start": 181913309, "end": 181919574}, {"filename": "/GameData/textures/lq_conc/conc4_3.png", "start": 181919574, "end": 181926500}, {"filename": "/GameData/textures/lq_conc/conc4_4.png", "start": 181926500, "end": 181932990}, {"filename": "/GameData/textures/lq_conc/conc4_5.png", "start": 181932990, "end": 181939583}, {"filename": "/GameData/textures/lq_conc/conc4_6.png", "start": 181939583, "end": 181946107}, {"filename": "/GameData/textures/lq_conc/conc4_7.png", "start": 181946107, "end": 181952515}, {"filename": "/GameData/textures/lq_conc/conc4_8.png", "start": 181952515, "end": 181958798}, {"filename": "/GameData/textures/lq_conc/conc4_9.png", "start": 181958798, "end": 181965381}, {"filename": "/GameData/textures/lq_conc/conc4_a1.png", "start": 181965381, "end": 181971450}, {"filename": "/GameData/textures/lq_conc/conc5_1.png", "start": 181971450, "end": 181980212}, {"filename": "/GameData/textures/lq_conc/conc5_10.png", "start": 181980212, "end": 181989827}, {"filename": "/GameData/textures/lq_conc/conc5_2.png", "start": 181989827, "end": 181999143}, {"filename": "/GameData/textures/lq_conc/conc5_8.png", "start": 181999143, "end": 182008223}, {"filename": "/GameData/textures/lq_conc/conc5_9.png", "start": 182008223, "end": 182017937}, {"filename": "/GameData/textures/lq_conc/conc5_a1.png", "start": 182017937, "end": 182026312}, {"filename": "/GameData/textures/lq_conc/conc6_1.png", "start": 182026312, "end": 182032725}, {"filename": "/GameData/textures/lq_conc/conc6_10.png", "start": 182032725, "end": 182039134}, {"filename": "/GameData/textures/lq_conc/conc6_2.png", "start": 182039134, "end": 182045462}, {"filename": "/GameData/textures/lq_conc/conc6_3.png", "start": 182045462, "end": 182052323}, {"filename": "/GameData/textures/lq_conc/conc6_4.png", "start": 182052323, "end": 182058931}, {"filename": "/GameData/textures/lq_conc/conc6_5.png", "start": 182058931, "end": 182065607}, {"filename": "/GameData/textures/lq_conc/conc6_6.png", "start": 182065607, "end": 182072381}, {"filename": "/GameData/textures/lq_conc/conc6_7.png", "start": 182072381, "end": 182079084}, {"filename": "/GameData/textures/lq_conc/conc6_8.png", "start": 182079084, "end": 182085580}, {"filename": "/GameData/textures/lq_conc/conc6_9.png", "start": 182085580, "end": 182092202}, {"filename": "/GameData/textures/lq_conc/conc6_a1.png", "start": 182092202, "end": 182098846}, {"filename": "/GameData/textures/lq_conc/conc7_1.png", "start": 182098846, "end": 182106636}, {"filename": "/GameData/textures/lq_conc/conc7_10.png", "start": 182106636, "end": 182114314}, {"filename": "/GameData/textures/lq_conc/conc7_2.png", "start": 182114314, "end": 182121940}, {"filename": "/GameData/textures/lq_conc/conc7_3.png", "start": 182121940, "end": 182129884}, {"filename": "/GameData/textures/lq_conc/conc7_4.png", "start": 182129884, "end": 182137652}, {"filename": "/GameData/textures/lq_conc/conc7_5.png", "start": 182137652, "end": 182145476}, {"filename": "/GameData/textures/lq_conc/conc7_6.png", "start": 182145476, "end": 182153475}, {"filename": "/GameData/textures/lq_conc/conc7_7.png", "start": 182153475, "end": 182161455}, {"filename": "/GameData/textures/lq_conc/conc7_8.png", "start": 182161455, "end": 182169279}, {"filename": "/GameData/textures/lq_conc/conc7_9.png", "start": 182169279, "end": 182177079}, {"filename": "/GameData/textures/lq_conc/conc7_a1.png", "start": 182177079, "end": 182184812}, {"filename": "/GameData/textures/lq_conc/flr1_1.png", "start": 182184812, "end": 182187144}, {"filename": "/GameData/textures/lq_conc/flr1_2.png", "start": 182187144, "end": 182190408}, {"filename": "/GameData/textures/lq_conc/flr1_3.png", "start": 182190408, "end": 182193948}, {"filename": "/GameData/textures/lq_conc/flr1_4.png", "start": 182193948, "end": 182196725}, {"filename": "/GameData/textures/lq_conc/flr1_5.png", "start": 182196725, "end": 182200321}, {"filename": "/GameData/textures/lq_conc/flr1_6.png", "start": 182200321, "end": 182203377}, {"filename": "/GameData/textures/lq_conc/flr2_1.png", "start": 182203377, "end": 182205487}, {"filename": "/GameData/textures/lq_conc/flr2_2.png", "start": 182205487, "end": 182208700}, {"filename": "/GameData/textures/lq_conc/flr2_3.png", "start": 182208700, "end": 182212320}, {"filename": "/GameData/textures/lq_conc/flr2_4.png", "start": 182212320, "end": 182215718}, {"filename": "/GameData/textures/lq_conc/flr2_5.png", "start": 182215718, "end": 182219078}, {"filename": "/GameData/textures/lq_conc/flr2_6.png", "start": 182219078, "end": 182222277}, {"filename": "/GameData/textures/lq_conc/flr2_7.png", "start": 182222277, "end": 182225412}, {"filename": "/GameData/textures/lq_conc/flr2_8.png", "start": 182225412, "end": 182228353}, {"filename": "/GameData/textures/lq_dev/clip.png", "start": 182228353, "end": 182228859}, {"filename": "/GameData/textures/lq_dev/dot_blue_a.png", "start": 182228859, "end": 182229415}, {"filename": "/GameData/textures/lq_dev/dot_blue_b.png", "start": 182229415, "end": 182229970}, {"filename": "/GameData/textures/lq_dev/dot_blue_c.png", "start": 182229970, "end": 182230525}, {"filename": "/GameData/textures/lq_dev/dot_brown_a.png", "start": 182230525, "end": 182231080}, {"filename": "/GameData/textures/lq_dev/dot_brown_b.png", "start": 182231080, "end": 182231635}, {"filename": "/GameData/textures/lq_dev/dot_brown_c.png", "start": 182231635, "end": 182232190}, {"filename": "/GameData/textures/lq_dev/dot_green_a.png", "start": 182232190, "end": 182232746}, {"filename": "/GameData/textures/lq_dev/dot_green_b.png", "start": 182232746, "end": 182233301}, {"filename": "/GameData/textures/lq_dev/dot_green_c.png", "start": 182233301, "end": 182233856}, {"filename": "/GameData/textures/lq_dev/dot_grey_a.png", "start": 182233856, "end": 182234409}, {"filename": "/GameData/textures/lq_dev/dot_grey_b.png", "start": 182234409, "end": 182234965}, {"filename": "/GameData/textures/lq_dev/dot_grey_c.png", "start": 182234965, "end": 182235520}, {"filename": "/GameData/textures/lq_dev/dot_olive_a.png", "start": 182235520, "end": 182236075}, {"filename": "/GameData/textures/lq_dev/dot_olive_b.png", "start": 182236075, "end": 182236629}, {"filename": "/GameData/textures/lq_dev/dot_olive_c.png", "start": 182236629, "end": 182237184}, {"filename": "/GameData/textures/lq_dev/dot_orange_a.png", "start": 182237184, "end": 182237740}, {"filename": "/GameData/textures/lq_dev/dot_orange_b.png", "start": 182237740, "end": 182238295}, {"filename": "/GameData/textures/lq_dev/dot_orange_c.png", "start": 182238295, "end": 182238850}, {"filename": "/GameData/textures/lq_dev/dot_pink_a.png", "start": 182238850, "end": 182239406}, {"filename": "/GameData/textures/lq_dev/dot_pink_b.png", "start": 182239406, "end": 182239962}, {"filename": "/GameData/textures/lq_dev/dot_pink_c.png", "start": 182239962, "end": 182240517}, {"filename": "/GameData/textures/lq_dev/dot_purple_a.png", "start": 182240517, "end": 182241073}, {"filename": "/GameData/textures/lq_dev/dot_purple_b.png", "start": 182241073, "end": 182241628}, {"filename": "/GameData/textures/lq_dev/dot_purple_c.png", "start": 182241628, "end": 182242183}, {"filename": "/GameData/textures/lq_dev/dot_red_a.png", "start": 182242183, "end": 182242734}, {"filename": "/GameData/textures/lq_dev/dot_red_b.png", "start": 182242734, "end": 182243285}, {"filename": "/GameData/textures/lq_dev/dot_red_c.png", "start": 182243285, "end": 182243836}, {"filename": "/GameData/textures/lq_dev/dot_tan_a.png", "start": 182243836, "end": 182244392}, {"filename": "/GameData/textures/lq_dev/dot_tan_b.png", "start": 182244392, "end": 182244948}, {"filename": "/GameData/textures/lq_dev/dot_tan_c.png", "start": 182244948, "end": 182245503}, {"filename": "/GameData/textures/lq_dev/dot_yellow_a.png", "start": 182245503, "end": 182246059}, {"filename": "/GameData/textures/lq_dev/dot_yellow_b.png", "start": 182246059, "end": 182246615}, {"filename": "/GameData/textures/lq_dev/dot_yellow_c.png", "start": 182246615, "end": 182247169}, {"filename": "/GameData/textures/lq_dev/floor_blue_a.png", "start": 182247169, "end": 182247986}, {"filename": "/GameData/textures/lq_dev/floor_blue_b.png", "start": 182247986, "end": 182248804}, {"filename": "/GameData/textures/lq_dev/floor_blue_c.png", "start": 182248804, "end": 182249622}, {"filename": "/GameData/textures/lq_dev/floor_brown_a.png", "start": 182249622, "end": 182250444}, {"filename": "/GameData/textures/lq_dev/floor_brown_b.png", "start": 182250444, "end": 182251267}, {"filename": "/GameData/textures/lq_dev/floor_brown_c.png", "start": 182251267, "end": 182252089}, {"filename": "/GameData/textures/lq_dev/floor_green_a.png", "start": 182252089, "end": 182252910}, {"filename": "/GameData/textures/lq_dev/floor_green_b.png", "start": 182252910, "end": 182253732}, {"filename": "/GameData/textures/lq_dev/floor_green_c.png", "start": 182253732, "end": 182254554}, {"filename": "/GameData/textures/lq_dev/floor_grey_a.png", "start": 182254554, "end": 182255364}, {"filename": "/GameData/textures/lq_dev/floor_grey_b.png", "start": 182255364, "end": 182256177}, {"filename": "/GameData/textures/lq_dev/floor_grey_c.png", "start": 182256177, "end": 182256991}, {"filename": "/GameData/textures/lq_dev/floor_olive_a.png", "start": 182256991, "end": 182257809}, {"filename": "/GameData/textures/lq_dev/floor_olive_b.png", "start": 182257809, "end": 182258624}, {"filename": "/GameData/textures/lq_dev/floor_olive_c.png", "start": 182258624, "end": 182259441}, {"filename": "/GameData/textures/lq_dev/floor_orange_a.png", "start": 182259441, "end": 182260264}, {"filename": "/GameData/textures/lq_dev/floor_orange_b.png", "start": 182260264, "end": 182261087}, {"filename": "/GameData/textures/lq_dev/floor_orange_c.png", "start": 182261087, "end": 182261907}, {"filename": "/GameData/textures/lq_dev/floor_pink_a.png", "start": 182261907, "end": 182262730}, {"filename": "/GameData/textures/lq_dev/floor_pink_b.png", "start": 182262730, "end": 182263552}, {"filename": "/GameData/textures/lq_dev/floor_pink_c.png", "start": 182263552, "end": 182264374}, {"filename": "/GameData/textures/lq_dev/floor_purple_a.png", "start": 182264374, "end": 182265196}, {"filename": "/GameData/textures/lq_dev/floor_purple_b.png", "start": 182265196, "end": 182266018}, {"filename": "/GameData/textures/lq_dev/floor_purple_c.png", "start": 182266018, "end": 182266839}, {"filename": "/GameData/textures/lq_dev/floor_red_a.png", "start": 182266839, "end": 182267649}, {"filename": "/GameData/textures/lq_dev/floor_red_b.png", "start": 182267649, "end": 182268459}, {"filename": "/GameData/textures/lq_dev/floor_red_c.png", "start": 182268459, "end": 182269270}, {"filename": "/GameData/textures/lq_dev/floor_tan_a.png", "start": 182269270, "end": 182270090}, {"filename": "/GameData/textures/lq_dev/floor_tan_b.png", "start": 182270090, "end": 182270914}, {"filename": "/GameData/textures/lq_dev/floor_tan_c.png", "start": 182270914, "end": 182271735}, {"filename": "/GameData/textures/lq_dev/floor_yellow_a.png", "start": 182271735, "end": 182272556}, {"filename": "/GameData/textures/lq_dev/floor_yellow_b.png", "start": 182272556, "end": 182273378}, {"filename": "/GameData/textures/lq_dev/floor_yellow_c.png", "start": 182273378, "end": 182274197}, {"filename": "/GameData/textures/lq_dev/hint.png", "start": 182274197, "end": 182275063}, {"filename": "/GameData/textures/lq_dev/hintskip.png", "start": 182275063, "end": 182275956}, {"filename": "/GameData/textures/lq_dev/key_gold_1.png", "start": 182275956, "end": 182276711}, {"filename": "/GameData/textures/lq_dev/key_silver_1.png", "start": 182276711, "end": 182277465}, {"filename": "/GameData/textures/lq_dev/light_fbr.png", "start": 182277465, "end": 182278381}, {"filename": "/GameData/textures/lq_dev/origin.png", "start": 182278381, "end": 182278864}, {"filename": "/GameData/textures/lq_dev/plus_0_button_fbr.png", "start": 182278864, "end": 182279063}, {"filename": "/GameData/textures/lq_dev/plus_0_shoot_fbr.png", "start": 182279063, "end": 182279342}, {"filename": "/GameData/textures/lq_dev/plus_1_button_fbr.png", "start": 182279342, "end": 182279543}, {"filename": "/GameData/textures/lq_dev/plus_1_shoot_fbr.png", "start": 182279543, "end": 182279824}, {"filename": "/GameData/textures/lq_dev/plus_a_button_fbr.png", "start": 182279824, "end": 182280025}, {"filename": "/GameData/textures/lq_dev/plus_a_shoot_fbr.png", "start": 182280025, "end": 182280312}, {"filename": "/GameData/textures/lq_dev/skip.png", "start": 182280312, "end": 182280800}, {"filename": "/GameData/textures/lq_dev/sky_dev.png", "start": 182280800, "end": 182282772}, {"filename": "/GameData/textures/lq_dev/sky_dev.png.bak", "start": 182282772, "end": 182295126}, {"filename": "/GameData/textures/lq_dev/sky_dev_day_fbr.png", "start": 182295126, "end": 182298952}, {"filename": "/GameData/textures/lq_dev/sky_dev_void.png", "start": 182298952, "end": 182299863}, {"filename": "/GameData/textures/lq_dev/star_blood1.png", "start": 182299863, "end": 182300225}, {"filename": "/GameData/textures/lq_dev/star_lava1.png", "start": 182300225, "end": 182300596}, {"filename": "/GameData/textures/lq_dev/star_lavaskip.png", "start": 182300596, "end": 182301709}, {"filename": "/GameData/textures/lq_dev/star_slime1.png", "start": 182301709, "end": 182302079}, {"filename": "/GameData/textures/lq_dev/star_slimeskip.png", "start": 182302079, "end": 182303152}, {"filename": "/GameData/textures/lq_dev/star_smile.png", "start": 182303152, "end": 182303632}, {"filename": "/GameData/textures/lq_dev/star_teleport.png", "start": 182303632, "end": 182303956}, {"filename": "/GameData/textures/lq_dev/star_water1.png", "start": 182303956, "end": 182304328}, {"filename": "/GameData/textures/lq_dev/star_water2.png", "start": 182304328, "end": 182304697}, {"filename": "/GameData/textures/lq_dev/star_waterskip.png", "start": 182304697, "end": 182306437}, {"filename": "/GameData/textures/lq_dev/trigger.png", "start": 182306437, "end": 182306934}, {"filename": "/GameData/textures/lq_dev/wall_blue_a.png", "start": 182306934, "end": 182307614}, {"filename": "/GameData/textures/lq_dev/wall_blue_b.png", "start": 182307614, "end": 182308294}, {"filename": "/GameData/textures/lq_dev/wall_blue_c.png", "start": 182308294, "end": 182308974}, {"filename": "/GameData/textures/lq_dev/wall_brown_a.png", "start": 182308974, "end": 182309656}, {"filename": "/GameData/textures/lq_dev/wall_brown_b.png", "start": 182309656, "end": 182310338}, {"filename": "/GameData/textures/lq_dev/wall_brown_c.png", "start": 182310338, "end": 182311020}, {"filename": "/GameData/textures/lq_dev/wall_green_a.png", "start": 182311020, "end": 182311702}, {"filename": "/GameData/textures/lq_dev/wall_green_b.png", "start": 182311702, "end": 182312384}, {"filename": "/GameData/textures/lq_dev/wall_green_c.png", "start": 182312384, "end": 182313066}, {"filename": "/GameData/textures/lq_dev/wall_grey_a.png", "start": 182313066, "end": 182313740}, {"filename": "/GameData/textures/lq_dev/wall_grey_b.png", "start": 182313740, "end": 182314416}, {"filename": "/GameData/textures/lq_dev/wall_grey_c.png", "start": 182314416, "end": 182315093}, {"filename": "/GameData/textures/lq_dev/wall_olive_a.png", "start": 182315093, "end": 182315773}, {"filename": "/GameData/textures/lq_dev/wall_olive_b.png", "start": 182315773, "end": 182316450}, {"filename": "/GameData/textures/lq_dev/wall_olive_c.png", "start": 182316450, "end": 182317129}, {"filename": "/GameData/textures/lq_dev/wall_orange_a.png", "start": 182317129, "end": 182317811}, {"filename": "/GameData/textures/lq_dev/wall_orange_b.png", "start": 182317811, "end": 182318493}, {"filename": "/GameData/textures/lq_dev/wall_orange_c.png", "start": 182318493, "end": 182319174}, {"filename": "/GameData/textures/lq_dev/wall_pink_a.png", "start": 182319174, "end": 182319856}, {"filename": "/GameData/textures/lq_dev/wall_pink_b.png", "start": 182319856, "end": 182320538}, {"filename": "/GameData/textures/lq_dev/wall_pink_c.png", "start": 182320538, "end": 182321220}, {"filename": "/GameData/textures/lq_dev/wall_purple_a.png", "start": 182321220, "end": 182321903}, {"filename": "/GameData/textures/lq_dev/wall_purple_b.png", "start": 182321903, "end": 182322585}, {"filename": "/GameData/textures/lq_dev/wall_purple_c.png", "start": 182322585, "end": 182323267}, {"filename": "/GameData/textures/lq_dev/wall_red_a.png", "start": 182323267, "end": 182323940}, {"filename": "/GameData/textures/lq_dev/wall_red_b.png", "start": 182323940, "end": 182324613}, {"filename": "/GameData/textures/lq_dev/wall_red_c.png", "start": 182324613, "end": 182325287}, {"filename": "/GameData/textures/lq_dev/wall_tan_a.png", "start": 182325287, "end": 182325969}, {"filename": "/GameData/textures/lq_dev/wall_tan_b.png", "start": 182325969, "end": 182326651}, {"filename": "/GameData/textures/lq_dev/wall_tan_c.png", "start": 182326651, "end": 182327333}, {"filename": "/GameData/textures/lq_dev/wall_yellow_a.png", "start": 182327333, "end": 182328015}, {"filename": "/GameData/textures/lq_dev/wall_yellow_b.png", "start": 182328015, "end": 182328697}, {"filename": "/GameData/textures/lq_dev/wall_yellow_c.png", "start": 182328697, "end": 182329378}, {"filename": "/GameData/textures/lq_dev/{char_0_fbr.png", "start": 182329378, "end": 182329555}, {"filename": "/GameData/textures/lq_dev/{char_1_fbr.png", "start": 182329555, "end": 182329715}, {"filename": "/GameData/textures/lq_dev/{char_2_fbr.png", "start": 182329715, "end": 182329887}, {"filename": "/GameData/textures/lq_dev/{char_3_fbr.png", "start": 182329887, "end": 182330066}, {"filename": "/GameData/textures/lq_dev/{char_4_fbr.png", "start": 182330066, "end": 182330229}, {"filename": "/GameData/textures/lq_dev/{char_5_fbr.png", "start": 182330229, "end": 182330402}, {"filename": "/GameData/textures/lq_dev/{char_6_fbr.png", "start": 182330402, "end": 182330574}, {"filename": "/GameData/textures/lq_dev/{char_7_fbr.png", "start": 182330574, "end": 182330728}, {"filename": "/GameData/textures/lq_dev/{char_8_fbr.png", "start": 182330728, "end": 182330896}, {"filename": "/GameData/textures/lq_dev/{char_9_fbr.png", "start": 182330896, "end": 182331063}, {"filename": "/GameData/textures/lq_dev/{char_a_fbr.png", "start": 182331063, "end": 182331227}, {"filename": "/GameData/textures/lq_dev/{char_b_fbr.png", "start": 182331227, "end": 182331397}, {"filename": "/GameData/textures/lq_dev/{char_c_fbr.png", "start": 182331397, "end": 182331547}, {"filename": "/GameData/textures/lq_dev/{char_d_fbr.png", "start": 182331547, "end": 182331713}, {"filename": "/GameData/textures/lq_dev/{char_e_fbr.png", "start": 182331713, "end": 182331881}, {"filename": "/GameData/textures/lq_dev/{char_f_fbr.png", "start": 182331881, "end": 182332041}, {"filename": "/GameData/textures/lq_dev/{char_g_fbr.png", "start": 182332041, "end": 182332213}, {"filename": "/GameData/textures/lq_dev/{char_h_fbr.png", "start": 182332213, "end": 182332372}, {"filename": "/GameData/textures/lq_dev/{char_i_fbr.png", "start": 182332372, "end": 182332531}, {"filename": "/GameData/textures/lq_dev/{char_j_fbr.png", "start": 182332531, "end": 182332697}, {"filename": "/GameData/textures/lq_dev/{char_k_fbr.png", "start": 182332697, "end": 182332891}, {"filename": "/GameData/textures/lq_dev/{char_l_fbr.png", "start": 182332891, "end": 182333032}, {"filename": "/GameData/textures/lq_dev/{char_m_fbr.png", "start": 182333032, "end": 182333181}, {"filename": "/GameData/textures/lq_dev/{char_n_fbr.png", "start": 182333181, "end": 182333364}, {"filename": "/GameData/textures/lq_dev/{char_o_fbr.png", "start": 182333364, "end": 182333514}, {"filename": "/GameData/textures/lq_dev/{char_p_fbr.png", "start": 182333514, "end": 182333672}, {"filename": "/GameData/textures/lq_dev/{char_q_fbr.png", "start": 182333672, "end": 182333838}, {"filename": "/GameData/textures/lq_dev/{char_r_fbr.png", "start": 182333838, "end": 182334010}, {"filename": "/GameData/textures/lq_dev/{char_s_fbr.png", "start": 182334010, "end": 182334186}, {"filename": "/GameData/textures/lq_dev/{char_t_fbr.png", "start": 182334186, "end": 182334334}, {"filename": "/GameData/textures/lq_dev/{char_trans_fbr.png", "start": 182334334, "end": 182334457}, {"filename": "/GameData/textures/lq_dev/{char_u_fbr.png", "start": 182334457, "end": 182334603}, {"filename": "/GameData/textures/lq_dev/{char_v_fbr.png", "start": 182334603, "end": 182334771}, {"filename": "/GameData/textures/lq_dev/{char_w_fbr.png", "start": 182334771, "end": 182334920}, {"filename": "/GameData/textures/lq_dev/{char_x_fbr.png", "start": 182334920, "end": 182335102}, {"filename": "/GameData/textures/lq_dev/{char_y_fbr.png", "start": 182335102, "end": 182335271}, {"filename": "/GameData/textures/lq_dev/{char_z_fbr.png", "start": 182335271, "end": 182335446}, {"filename": "/GameData/textures/lq_dev/{charlow_a_fbr.png", "start": 182335446, "end": 182335601}, {"filename": "/GameData/textures/lq_dev/{charlow_b_fbr.png", "start": 182335601, "end": 182335760}, {"filename": "/GameData/textures/lq_dev/{charlow_c_fbr.png", "start": 182335760, "end": 182335914}, {"filename": "/GameData/textures/lq_dev/{charlow_d_fbr.png", "start": 182335914, "end": 182336075}, {"filename": "/GameData/textures/lq_dev/{charlow_e_fbr.png", "start": 182336075, "end": 182336229}, {"filename": "/GameData/textures/lq_dev/{charlow_f_fbr.png", "start": 182336229, "end": 182336389}, {"filename": "/GameData/textures/lq_dev/{charlow_g_fbr.png", "start": 182336389, "end": 182336547}, {"filename": "/GameData/textures/lq_dev/{charlow_h_fbr.png", "start": 182336547, "end": 182336701}, {"filename": "/GameData/textures/lq_dev/{charlow_i_fbr.png", "start": 182336701, "end": 182336847}, {"filename": "/GameData/textures/lq_dev/{charlow_j_fbr.png", "start": 182336847, "end": 182337007}, {"filename": "/GameData/textures/lq_dev/{charlow_k_fbr.png", "start": 182337007, "end": 182337186}, {"filename": "/GameData/textures/lq_dev/{charlow_l_fbr.png", "start": 182337186, "end": 182337324}, {"filename": "/GameData/textures/lq_dev/{charlow_m_fbr.png", "start": 182337324, "end": 182337477}, {"filename": "/GameData/textures/lq_dev/{charlow_n_fbr.png", "start": 182337477, "end": 182337627}, {"filename": "/GameData/textures/lq_dev/{charlow_o_fbr.png", "start": 182337627, "end": 182337781}, {"filename": "/GameData/textures/lq_dev/{charlow_p_fbr.png", "start": 182337781, "end": 182337936}, {"filename": "/GameData/textures/lq_dev/{charlow_q_fbr.png", "start": 182337936, "end": 182338086}, {"filename": "/GameData/textures/lq_dev/{charlow_r_fbr.png", "start": 182338086, "end": 182338239}, {"filename": "/GameData/textures/lq_dev/{charlow_s_fbr.png", "start": 182338239, "end": 182338391}, {"filename": "/GameData/textures/lq_dev/{charlow_t_fbr.png", "start": 182338391, "end": 182338557}, {"filename": "/GameData/textures/lq_dev/{charlow_u_fbr.png", "start": 182338557, "end": 182338708}, {"filename": "/GameData/textures/lq_dev/{charlow_v_fbr.png", "start": 182338708, "end": 182338882}, {"filename": "/GameData/textures/lq_dev/{charlow_w_fbr.png", "start": 182338882, "end": 182339036}, {"filename": "/GameData/textures/lq_dev/{charlow_x_fbr.png", "start": 182339036, "end": 182339223}, {"filename": "/GameData/textures/lq_dev/{charlow_y_fbr.png", "start": 182339223, "end": 182339397}, {"filename": "/GameData/textures/lq_dev/{charlow_z_fbr.png", "start": 182339397, "end": 182339568}, {"filename": "/GameData/textures/lq_dev/{chars_add_fbr.png", "start": 182339568, "end": 182339731}, {"filename": "/GameData/textures/lq_dev/{chars_and_fbr.png", "start": 182339731, "end": 182339923}, {"filename": "/GameData/textures/lq_dev/{chars_ardown_fbr.png", "start": 182339923, "end": 182340108}, {"filename": "/GameData/textures/lq_dev/{chars_arleft_fbr.png", "start": 182340108, "end": 182340292}, {"filename": "/GameData/textures/lq_dev/{chars_arright_fbr.png", "start": 182340292, "end": 182340477}, {"filename": "/GameData/textures/lq_dev/{chars_arup_fbr.png", "start": 182340477, "end": 182340655}, {"filename": "/GameData/textures/lq_dev/{chars_at_fbr.png", "start": 182340655, "end": 182340812}, {"filename": "/GameData/textures/lq_dev/{chars_brackc1_fbr.png", "start": 182340812, "end": 182340991}, {"filename": "/GameData/textures/lq_dev/{chars_brackc2_fbr.png", "start": 182340991, "end": 182341175}, {"filename": "/GameData/textures/lq_dev/{chars_brackr1_fbr.png", "start": 182341175, "end": 182341342}, {"filename": "/GameData/textures/lq_dev/{chars_brackr2_fbr.png", "start": 182341342, "end": 182341512}, {"filename": "/GameData/textures/lq_dev/{chars_bracks1_fbr.png", "start": 182341512, "end": 182341666}, {"filename": "/GameData/textures/lq_dev/{chars_bracks2_fbr.png", "start": 182341666, "end": 182341819}, {"filename": "/GameData/textures/lq_dev/{chars_caret_fbr.png", "start": 182341819, "end": 182341999}, {"filename": "/GameData/textures/lq_dev/{chars_colon_fbr.png", "start": 182341999, "end": 182342151}, {"filename": "/GameData/textures/lq_dev/{chars_colonsemi_fbr.png", "start": 182342151, "end": 182342318}, {"filename": "/GameData/textures/lq_dev/{chars_comma_fbr.png", "start": 182342318, "end": 182342474}, {"filename": "/GameData/textures/lq_dev/{chars_div_fbr.png", "start": 182342474, "end": 182342643}, {"filename": "/GameData/textures/lq_dev/{chars_dollar_fbr.png", "start": 182342643, "end": 182342819}, {"filename": "/GameData/textures/lq_dev/{chars_equ_fbr.png", "start": 182342819, "end": 182342978}, {"filename": "/GameData/textures/lq_dev/{chars_exclam_fbr.png", "start": 182342978, "end": 182343122}, {"filename": "/GameData/textures/lq_dev/{chars_grave_fbr.png", "start": 182343122, "end": 182343272}, {"filename": "/GameData/textures/lq_dev/{chars_hash_fbr.png", "start": 182343272, "end": 182343457}, {"filename": "/GameData/textures/lq_dev/{chars_heart_fbr.png", "start": 182343457, "end": 182343642}, {"filename": "/GameData/textures/lq_dev/{chars_multi_fbr.png", "start": 182343642, "end": 182343811}, {"filename": "/GameData/textures/lq_dev/{chars_percent_fbr.png", "start": 182343811, "end": 182344017}, {"filename": "/GameData/textures/lq_dev/{chars_perio_fbr.png", "start": 182344017, "end": 182344154}, {"filename": "/GameData/textures/lq_dev/{chars_pipe_fbr.png", "start": 182344154, "end": 182344301}, {"filename": "/GameData/textures/lq_dev/{chars_quest_fbr.png", "start": 182344301, "end": 182344474}, {"filename": "/GameData/textures/lq_dev/{chars_slaback_fbr.png", "start": 182344474, "end": 182344661}, {"filename": "/GameData/textures/lq_dev/{chars_slafoward_fbr.png", "start": 182344661, "end": 182344840}, {"filename": "/GameData/textures/lq_dev/{chars_smile_fbr.png", "start": 182344840, "end": 182345000}, {"filename": "/GameData/textures/lq_dev/{chars_sub_fbr.png", "start": 182345000, "end": 182345147}, {"filename": "/GameData/textures/lq_dev/{chars_sun_fbr.png", "start": 182345147, "end": 182345346}, {"filename": "/GameData/textures/lq_dev/{chars_thngreater_fbr.png", "start": 182345346, "end": 182345539}, {"filename": "/GameData/textures/lq_dev/{chars_thnless_fbr.png", "start": 182345539, "end": 182345726}, {"filename": "/GameData/textures/lq_dev/{chars_tilde_fbr.png", "start": 182345726, "end": 182345895}, {"filename": "/GameData/textures/lq_dev/{chars_unders_fbr.png", "start": 182345895, "end": 182346032}, {"filename": "/GameData/textures/lq_flesh/bone1_1.png", "start": 182346032, "end": 182353439}, {"filename": "/GameData/textures/lq_flesh/bone1_2.png", "start": 182353439, "end": 182362631}, {"filename": "/GameData/textures/lq_flesh/bone2_1.png", "start": 182362631, "end": 182373063}, {"filename": "/GameData/textures/lq_flesh/dopefish_fbr.png", "start": 182373063, "end": 182381293}, {"filename": "/GameData/textures/lq_flesh/flesh_gut1.png", "start": 182381293, "end": 182391608}, {"filename": "/GameData/textures/lq_flesh/flesh_gut2.png", "start": 182391608, "end": 182403408}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_1.png", "start": 182403408, "end": 182417158}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_2.png", "start": 182417158, "end": 182430138}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_3.png", "start": 182430138, "end": 182442171}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_4a.png", "start": 182442171, "end": 182455384}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_4b.png", "start": 182455384, "end": 182468524}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_5a.png", "start": 182468524, "end": 182482168}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_5b.png", "start": 182482168, "end": 182495741}, {"filename": "/GameData/textures/lq_flesh/flesh_rot2_1.png", "start": 182495741, "end": 182508967}, {"filename": "/GameData/textures/lq_flesh/flesh_rot3_1.png", "start": 182508967, "end": 182523913}, {"filename": "/GameData/textures/lq_flesh/flesh_rot3_2.png", "start": 182523913, "end": 182538952}, {"filename": "/GameData/textures/lq_flesh/flesh_rot4_1.png", "start": 182538952, "end": 182553589}, {"filename": "/GameData/textures/lq_flesh/flesh_rot5_1.png", "start": 182553589, "end": 182565857}, {"filename": "/GameData/textures/lq_flesh/flesh_rot6_1.png", "start": 182565857, "end": 182582264}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_1.png", "start": 182582264, "end": 182593782}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_2.png", "start": 182593782, "end": 182606844}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_3.png", "start": 182606844, "end": 182617847}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_4.png", "start": 182617847, "end": 182629259}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_5.png", "start": 182629259, "end": 182639858}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_1.png", "start": 182639858, "end": 182643514}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_2.png", "start": 182643514, "end": 182647611}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_3.png", "start": 182647611, "end": 182650987}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_4.png", "start": 182650987, "end": 182654585}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_5.png", "start": 182654585, "end": 182657975}, {"filename": "/GameData/textures/lq_flesh/fleshtile.png", "start": 182657975, "end": 182668944}, {"filename": "/GameData/textures/lq_flesh/marbred128.png", "start": 182668944, "end": 182681173}, {"filename": "/GameData/textures/lq_flesh/may_eat_eye2_fbr.png", "start": 182681173, "end": 182685046}, {"filename": "/GameData/textures/lq_flesh/may_eat_eye3_fbr.png", "start": 182685046, "end": 182688923}, {"filename": "/GameData/textures/lq_flesh/may_eat_eye_fbr.png", "start": 182688923, "end": 182692798}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_b.png", "start": 182692798, "end": 182705461}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_c.png", "start": 182705461, "end": 182718185}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_hol1.png", "start": 182718185, "end": 182722857}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_lit1_fbr.png", "start": 182722857, "end": 182726927}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_shut1.png", "start": 182726927, "end": 182741893}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_sp.png", "start": 182741893, "end": 182762035}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_sp2.png", "start": 182762035, "end": 182787658}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_te.png", "start": 182787658, "end": 182802764}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_tet.png", "start": 182802764, "end": 182815984}, {"filename": "/GameData/textures/lq_flesh/may_flesh2_b.png", "start": 182815984, "end": 182829507}, {"filename": "/GameData/textures/lq_flesh/may_flesh3_1a.png", "start": 182829507, "end": 182843233}, {"filename": "/GameData/textures/lq_flesh/may_flesh3_1b.png", "start": 182843233, "end": 182879795}, {"filename": "/GameData/textures/lq_flesh/may_flesh3_b.png", "start": 182879795, "end": 182893681}, {"filename": "/GameData/textures/lq_flesh/may_flesh4_det.png", "start": 182893681, "end": 182903992}, {"filename": "/GameData/textures/lq_flesh/may_flesh4a_det.png", "start": 182903992, "end": 182918853}, {"filename": "/GameData/textures/lq_flesh/may_flesh5.png", "start": 182918853, "end": 182929189}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1a.png", "start": 182929189, "end": 182983924}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1b.png", "start": 182983924, "end": 183036642}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1c.png", "start": 183036642, "end": 183080284}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1lit_fbr.png", "start": 183080284, "end": 183097218}, {"filename": "/GameData/textures/lq_flesh/may_flesh_dr1a.png", "start": 183097218, "end": 183154111}, {"filename": "/GameData/textures/lq_flesh/may_skin_eye.png", "start": 183154111, "end": 183165328}, {"filename": "/GameData/textures/lq_flesh/meat-teeth0.png", "start": 183165328, "end": 183208699}, {"filename": "/GameData/textures/lq_flesh/meat-teeth1.png", "start": 183208699, "end": 183253891}, {"filename": "/GameData/textures/lq_flesh/meat_det1.png", "start": 183253891, "end": 183272622}, {"filename": "/GameData/textures/lq_flesh/meat_det2.png", "start": 183272622, "end": 183295193}, {"filename": "/GameData/textures/lq_flesh/meat_pipe1.png", "start": 183295193, "end": 183319617}, {"filename": "/GameData/textures/lq_flesh/plus_0eye.png", "start": 183319617, "end": 183323762}, {"filename": "/GameData/textures/lq_flesh/plus_0flesh2_gl.png", "start": 183323762, "end": 183338942}, {"filename": "/GameData/textures/lq_flesh/plus_0flesh_but1_fbr.png", "start": 183338942, "end": 183343237}, {"filename": "/GameData/textures/lq_flesh/plus_0flesh_but2_fbr.png", "start": 183343237, "end": 183347426}, {"filename": "/GameData/textures/lq_flesh/plus_0flsh_vent.png", "start": 183347426, "end": 183361677}, {"filename": "/GameData/textures/lq_flesh/plus_1eye.png", "start": 183361677, "end": 183365821}, {"filename": "/GameData/textures/lq_flesh/plus_1flesh2_gl.png", "start": 183365821, "end": 183380980}, {"filename": "/GameData/textures/lq_flesh/plus_1flesh_but2_fbr.png", "start": 183380980, "end": 183385175}, {"filename": "/GameData/textures/lq_flesh/plus_2eye.png", "start": 183385175, "end": 183389215}, {"filename": "/GameData/textures/lq_flesh/plus_2flesh2_gl.png", "start": 183389215, "end": 183404391}, {"filename": "/GameData/textures/lq_flesh/plus_2flesh_but2_fbr.png", "start": 183404391, "end": 183408574}, {"filename": "/GameData/textures/lq_flesh/plus_3eye.png", "start": 183408574, "end": 183412622}, {"filename": "/GameData/textures/lq_flesh/plus_3flesh2_gl.png", "start": 183412622, "end": 183427796}, {"filename": "/GameData/textures/lq_flesh/plus_3flesh_but2_fbr.png", "start": 183427796, "end": 183431967}, {"filename": "/GameData/textures/lq_flesh/plus_4eye.png", "start": 183431967, "end": 183436086}, {"filename": "/GameData/textures/lq_flesh/plus_4flesh2_gl.png", "start": 183436086, "end": 183451266}, {"filename": "/GameData/textures/lq_flesh/plus_5eye.png", "start": 183451266, "end": 183455319}, {"filename": "/GameData/textures/lq_flesh/plus_6eye.png", "start": 183455319, "end": 183459367}, {"filename": "/GameData/textures/lq_flesh/plus_7eye.png", "start": 183459367, "end": 183463438}, {"filename": "/GameData/textures/lq_flesh/plus_8eye.png", "start": 183463438, "end": 183467546}, {"filename": "/GameData/textures/lq_flesh/plus_9eye.png", "start": 183467546, "end": 183471694}, {"filename": "/GameData/textures/lq_flesh/plus_aeye.png", "start": 183471694, "end": 183475686}, {"filename": "/GameData/textures/lq_flesh/plus_aflesh_but1.png", "start": 183475686, "end": 183480252}, {"filename": "/GameData/textures/lq_flesh/plus_aflesh_but2.png", "start": 183480252, "end": 183484781}, {"filename": "/GameData/textures/lq_flesh/plus_aflsh_vent.png", "start": 183484781, "end": 183499181}, {"filename": "/GameData/textures/lq_greek/black.png", "start": 183499181, "end": 183499326}, {"filename": "/GameData/textures/lq_greek/grk_arch1.png", "start": 183499326, "end": 183504343}, {"filename": "/GameData/textures/lq_greek/grk_arch1_2.png", "start": 183504343, "end": 183509969}, {"filename": "/GameData/textures/lq_greek/grk_arch1_a.png", "start": 183509969, "end": 183522744}, {"filename": "/GameData/textures/lq_greek/grk_arch1_b.png", "start": 183522744, "end": 183536204}, {"filename": "/GameData/textures/lq_greek/grk_arch1_c.png", "start": 183536204, "end": 183549377}, {"filename": "/GameData/textures/lq_greek/grk_arch1_d.png", "start": 183549377, "end": 183562894}, {"filename": "/GameData/textures/lq_greek/grk_arch2.png", "start": 183562894, "end": 183566176}, {"filename": "/GameData/textures/lq_greek/grk_arch2_2.png", "start": 183566176, "end": 183569498}, {"filename": "/GameData/textures/lq_greek/grk_arch2_a.png", "start": 183569498, "end": 183579901}, {"filename": "/GameData/textures/lq_greek/grk_arch2_b.png", "start": 183579901, "end": 183590418}, {"filename": "/GameData/textures/lq_greek/grk_arch2_c.png", "start": 183590418, "end": 183600547}, {"filename": "/GameData/textures/lq_greek/grk_arch_end.png", "start": 183600547, "end": 183601303}, {"filename": "/GameData/textures/lq_greek/grk_arch_tcap.png", "start": 183601303, "end": 183602017}, {"filename": "/GameData/textures/lq_greek/grk_arch_trim.png", "start": 183602017, "end": 183603489}, {"filename": "/GameData/textures/lq_greek/grk_bl_arch1a.png", "start": 183603489, "end": 183611124}, {"filename": "/GameData/textures/lq_greek/grk_bl_arch1b.png", "start": 183611124, "end": 183619659}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk1a.png", "start": 183619659, "end": 183622131}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk1b.png", "start": 183622131, "end": 183624269}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk2a.png", "start": 183624269, "end": 183626690}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk2b.png", "start": 183626690, "end": 183629150}, {"filename": "/GameData/textures/lq_greek/grk_bl_flt1.png", "start": 183629150, "end": 183631374}, {"filename": "/GameData/textures/lq_greek/grk_bl_fsh1.png", "start": 183631374, "end": 183634300}, {"filename": "/GameData/textures/lq_greek/grk_bl_fsh2.png", "start": 183634300, "end": 183642392}, {"filename": "/GameData/textures/lq_greek/grk_bl_pil1.png", "start": 183642392, "end": 183645179}, {"filename": "/GameData/textures/lq_greek/grk_bl_trim1.png", "start": 183645179, "end": 183647591}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll1.png", "start": 183647591, "end": 183650311}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll2.png", "start": 183650311, "end": 183653037}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll3a.png", "start": 183653037, "end": 183655395}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll3b.png", "start": 183655395, "end": 183657863}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll4b.png", "start": 183657863, "end": 183660429}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5a.png", "start": 183660429, "end": 183662646}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5b.png", "start": 183662646, "end": 183664945}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5c.png", "start": 183664945, "end": 183667268}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5d.png", "start": 183667268, "end": 183669675}, {"filename": "/GameData/textures/lq_greek/grk_brk15.png", "start": 183669675, "end": 183679181}, {"filename": "/GameData/textures/lq_greek/grk_brk15_b.png", "start": 183679181, "end": 183688429}, {"filename": "/GameData/textures/lq_greek/grk_brk15_c.png", "start": 183688429, "end": 183705297}, {"filename": "/GameData/textures/lq_greek/grk_brk15_f.png", "start": 183705297, "end": 183713590}, {"filename": "/GameData/textures/lq_greek/grk_brk15_g.png", "start": 183713590, "end": 183721239}, {"filename": "/GameData/textures/lq_greek/grk_brk16.png", "start": 183721239, "end": 183730515}, {"filename": "/GameData/textures/lq_greek/grk_brk16_a.png", "start": 183730515, "end": 183739794}, {"filename": "/GameData/textures/lq_greek/grk_brk16_f.png", "start": 183739794, "end": 183749499}, {"filename": "/GameData/textures/lq_greek/grk_brk17.png", "start": 183749499, "end": 183784643}, {"filename": "/GameData/textures/lq_greek/grk_brk17_f.png", "start": 183784643, "end": 183797245}, {"filename": "/GameData/textures/lq_greek/grk_but1.png", "start": 183797245, "end": 183798194}, {"filename": "/GameData/textures/lq_greek/grk_det1.png", "start": 183798194, "end": 183801267}, {"filename": "/GameData/textures/lq_greek/grk_door1.png", "start": 183801267, "end": 183816374}, {"filename": "/GameData/textures/lq_greek/grk_door1_f.png", "start": 183816374, "end": 183826409}, {"filename": "/GameData/textures/lq_greek/grk_door2.png", "start": 183826409, "end": 183837269}, {"filename": "/GameData/textures/lq_greek/grk_door3.png", "start": 183837269, "end": 183849926}, {"filename": "/GameData/textures/lq_greek/grk_ebrick10.png", "start": 183849926, "end": 183860553}, {"filename": "/GameData/textures/lq_greek/grk_ebrick10_bl.png", "start": 183860553, "end": 183875087}, {"filename": "/GameData/textures/lq_greek/grk_ebrick22.png", "start": 183875087, "end": 183888953}, {"filename": "/GameData/textures/lq_greek/grk_ebrick23.png", "start": 183888953, "end": 183898319}, {"filename": "/GameData/textures/lq_greek/grk_ebrick24.png", "start": 183898319, "end": 183906512}, {"filename": "/GameData/textures/lq_greek/grk_flr1.png", "start": 183906512, "end": 183906843}, {"filename": "/GameData/textures/lq_greek/grk_flr2.png", "start": 183906843, "end": 183907697}, {"filename": "/GameData/textures/lq_greek/grk_flr3.png", "start": 183907697, "end": 183910293}, {"filename": "/GameData/textures/lq_greek/grk_flr4_1.png", "start": 183910293, "end": 183912423}, {"filename": "/GameData/textures/lq_greek/grk_flr4_2.png", "start": 183912423, "end": 183914685}, {"filename": "/GameData/textures/lq_greek/grk_flr4_3.png", "start": 183914685, "end": 183917048}, {"filename": "/GameData/textures/lq_greek/grk_flr4_4.png", "start": 183917048, "end": 183919343}, {"filename": "/GameData/textures/lq_greek/grk_flr4_5.png", "start": 183919343, "end": 183922975}, {"filename": "/GameData/textures/lq_greek/grk_flr4_6.png", "start": 183922975, "end": 183925981}, {"filename": "/GameData/textures/lq_greek/grk_flr4_8.png", "start": 183925981, "end": 183929316}, {"filename": "/GameData/textures/lq_greek/grk_flr5_1.png", "start": 183929316, "end": 183931903}, {"filename": "/GameData/textures/lq_greek/grk_flr5_2.png", "start": 183931903, "end": 183934397}, {"filename": "/GameData/textures/lq_greek/grk_flr5_3.png", "start": 183934397, "end": 183937139}, {"filename": "/GameData/textures/lq_greek/grk_flr5_4.png", "start": 183937139, "end": 183939442}, {"filename": "/GameData/textures/lq_greek/grk_key_gl2.png", "start": 183939442, "end": 183940466}, {"filename": "/GameData/textures/lq_greek/grk_key_sl2.png", "start": 183940466, "end": 183941467}, {"filename": "/GameData/textures/lq_greek/grk_lion1.png", "start": 183941467, "end": 183951941}, {"filename": "/GameData/textures/lq_greek/grk_lion2.png", "start": 183951941, "end": 183961608}, {"filename": "/GameData/textures/lq_greek/grk_lion3.png", "start": 183961608, "end": 183972151}, {"filename": "/GameData/textures/lq_greek/grk_lion4.png", "start": 183972151, "end": 183983303}, {"filename": "/GameData/textures/lq_greek/grk_met1.png", "start": 183983303, "end": 183993684}, {"filename": "/GameData/textures/lq_greek/grk_met1_trim.png", "start": 183993684, "end": 184004959}, {"filename": "/GameData/textures/lq_greek/grk_met1a_trim.png", "start": 184004959, "end": 184015927}, {"filename": "/GameData/textures/lq_greek/grk_met1b_trim.png", "start": 184015927, "end": 184030040}, {"filename": "/GameData/textures/lq_greek/grk_met2_trim.png", "start": 184030040, "end": 184043309}, {"filename": "/GameData/textures/lq_greek/grk_met_plt.png", "start": 184043309, "end": 184058700}, {"filename": "/GameData/textures/lq_greek/grk_mural1.png", "start": 184058700, "end": 184078558}, {"filename": "/GameData/textures/lq_greek/grk_mural2.png", "start": 184078558, "end": 184101275}, {"filename": "/GameData/textures/lq_greek/grk_mural3.png", "start": 184101275, "end": 184190555}, {"filename": "/GameData/textures/lq_greek/grk_pl1_a.png", "start": 184190555, "end": 184192371}, {"filename": "/GameData/textures/lq_greek/grk_pl1_b.png", "start": 184192371, "end": 184194454}, {"filename": "/GameData/textures/lq_greek/grk_pl2_a.png", "start": 184194454, "end": 184196686}, {"filename": "/GameData/textures/lq_greek/grk_pl2_b.png", "start": 184196686, "end": 184198898}, {"filename": "/GameData/textures/lq_greek/grk_plat1_side.png", "start": 184198898, "end": 184200115}, {"filename": "/GameData/textures/lq_greek/grk_plat1_top.png", "start": 184200115, "end": 184204012}, {"filename": "/GameData/textures/lq_greek/grk_tile2_1.png", "start": 184204012, "end": 184206446}, {"filename": "/GameData/textures/lq_greek/grk_tile2_2.png", "start": 184206446, "end": 184208799}, {"filename": "/GameData/textures/lq_greek/grk_trim1.png", "start": 184208799, "end": 184215556}, {"filename": "/GameData/textures/lq_greek/grk_trim1_3.png", "start": 184215556, "end": 184222170}, {"filename": "/GameData/textures/lq_greek/grk_trim1_3_s.png", "start": 184222170, "end": 184224210}, {"filename": "/GameData/textures/lq_greek/grk_trim1_4_s.png", "start": 184224210, "end": 184226286}, {"filename": "/GameData/textures/lq_greek/grk_trim1_5.png", "start": 184226286, "end": 184233222}, {"filename": "/GameData/textures/lq_greek/grk_trim1_5_s.png", "start": 184233222, "end": 184235442}, {"filename": "/GameData/textures/lq_greek/grk_trim1_6_s.png", "start": 184235442, "end": 184237719}, {"filename": "/GameData/textures/lq_greek/grk_trim1_7_s.png", "start": 184237719, "end": 184241061}, {"filename": "/GameData/textures/lq_greek/grk_trim2.png", "start": 184241061, "end": 184243422}, {"filename": "/GameData/textures/lq_greek/grk_wall1.png", "start": 184243422, "end": 184246454}, {"filename": "/GameData/textures/lq_greek/grk_wall2.png", "start": 184246454, "end": 184249125}, {"filename": "/GameData/textures/lq_greek/grk_wall3.png", "start": 184249125, "end": 184254911}, {"filename": "/GameData/textures/lq_greek/grk_wall3b.png", "start": 184254911, "end": 184281945}, {"filename": "/GameData/textures/lq_greek/grk_win1_a.png", "start": 184281945, "end": 184291084}, {"filename": "/GameData/textures/lq_greek/grk_win1_b.png", "start": 184291084, "end": 184300427}, {"filename": "/GameData/textures/lq_greek/plus_0grk_but1_fbr.png", "start": 184300427, "end": 184301618}, {"filename": "/GameData/textures/lq_greek/plus_0grk_hbut_fbr.png", "start": 184301618, "end": 184302641}, {"filename": "/GameData/textures/lq_greek/plus_1grk_but1_fbr.png", "start": 184302641, "end": 184303816}, {"filename": "/GameData/textures/lq_greek/plus_1grk_hbut_fbr.png", "start": 184303816, "end": 184304897}, {"filename": "/GameData/textures/lq_greek/plus_2grk_but1_fbr.png", "start": 184304897, "end": 184306048}, {"filename": "/GameData/textures/lq_greek/plus_2grk_hbut_fbr.png", "start": 184306048, "end": 184307088}, {"filename": "/GameData/textures/lq_greek/plus_3grk_but1_fbr.png", "start": 184307088, "end": 184308241}, {"filename": "/GameData/textures/lq_greek/plus_3grk_hbut_fbr.png", "start": 184308241, "end": 184309283}, {"filename": "/GameData/textures/lq_greek/plus_agrk_but1.png", "start": 184309283, "end": 184310274}, {"filename": "/GameData/textures/lq_greek/plus_agrk_hbut.png", "start": 184310274, "end": 184311252}, {"filename": "/GameData/textures/lq_health_ammo/ammo_bottom.png", "start": 184311252, "end": 184312182}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_b1_fbr.png", "start": 184312182, "end": 184312790}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_b2_fbr.png", "start": 184312790, "end": 184313402}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_b3_fbr.png", "start": 184313402, "end": 184314209}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_s1_fbr.png", "start": 184314209, "end": 184314802}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_s2_fbr.png", "start": 184314802, "end": 184315792}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_s3_fbr.png", "start": 184315792, "end": 184316425}, {"filename": "/GameData/textures/lq_health_ammo/ammo_fl.png", "start": 184316425, "end": 184316984}, {"filename": "/GameData/textures/lq_health_ammo/ammo_fl2.png", "start": 184316984, "end": 184317539}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_b1_fbr.png", "start": 184317539, "end": 184318293}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_b2_fbr.png", "start": 184318293, "end": 184318978}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_b3.png", "start": 184318978, "end": 184319549}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_s1.png", "start": 184319549, "end": 184320120}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_s2_fbr.png", "start": 184320120, "end": 184320775}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_s3_fbr.png", "start": 184320775, "end": 184321312}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_b1_fbr.png", "start": 184321312, "end": 184322027}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_b2_fbr.png", "start": 184322027, "end": 184322653}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_b3_fbr.png", "start": 184322653, "end": 184323111}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_s1_fbr.png", "start": 184323111, "end": 184323725}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_s2_fbr.png", "start": 184323725, "end": 184324074}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_b1_fbr.png", "start": 184324074, "end": 184324733}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_b2_fbr.png", "start": 184324733, "end": 184325361}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_b3.png", "start": 184325361, "end": 184326138}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_s1_fbr.png", "start": 184326138, "end": 184326644}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_s2_fbr.png", "start": 184326644, "end": 184327222}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_s3_fbr.png", "start": 184327222, "end": 184327773}, {"filename": "/GameData/textures/lq_health_ammo/ammobotsmall.png", "start": 184327773, "end": 184328441}, {"filename": "/GameData/textures/lq_health_ammo/ammotop.png", "start": 184328441, "end": 184329186}, {"filename": "/GameData/textures/lq_health_ammo/ammotopsmall.png", "start": 184329186, "end": 184329737}, {"filename": "/GameData/textures/lq_health_ammo/boom.png", "start": 184329737, "end": 184330755}, {"filename": "/GameData/textures/lq_health_ammo/boomammo_bottom.png", "start": 184330755, "end": 184331121}, {"filename": "/GameData/textures/lq_health_ammo/boomammotop.png", "start": 184331121, "end": 184331463}, {"filename": "/GameData/textures/lq_health_ammo/boomsmall.png", "start": 184331463, "end": 184332138}, {"filename": "/GameData/textures/lq_health_ammo/epboxlarge_fbr.png", "start": 184332138, "end": 184333322}, {"filename": "/GameData/textures/lq_health_ammo/epboxsmall_fbr.png", "start": 184333322, "end": 184334219}, {"filename": "/GameData/textures/lq_health_ammo/explob_s2.png", "start": 184334219, "end": 184334715}, {"filename": "/GameData/textures/lq_health_ammo/hp15_side.png", "start": 184334715, "end": 184335300}, {"filename": "/GameData/textures/lq_health_ammo/hp25_top2.png", "start": 184335300, "end": 184335729}, {"filename": "/GameData/textures/lq_health_ammo/hp_bottom.png", "start": 184335729, "end": 184336636}, {"filename": "/GameData/textures/lq_health_ammo/hp_details.png", "start": 184336636, "end": 184337140}, {"filename": "/GameData/textures/lq_health_ammo/nails.png", "start": 184337140, "end": 184338187}, {"filename": "/GameData/textures/lq_health_ammo/nailssmall.png", "start": 184338187, "end": 184338928}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp100-winq_fbr.png", "start": 184338928, "end": 184341528}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp100_side_fbr.png", "start": 184341528, "end": 184342420}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp100_top_fbr.png", "start": 184342420, "end": 184343151}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp15_top_fbr.png", "start": 184343151, "end": 184343881}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp15winq_fbr.png", "start": 184343881, "end": 184346668}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp25-winq_fbr.png", "start": 184346668, "end": 184349222}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp25_side_fbr.png", "start": 184349222, "end": 184350044}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp25_top_fbr.png", "start": 184350044, "end": 184350778}, {"filename": "/GameData/textures/lq_health_ammo/plus_0explob2_s1_fbr.png", "start": 184350778, "end": 184351881}, {"filename": "/GameData/textures/lq_health_ammo/plus_0explob_s1_fbr.png", "start": 184351881, "end": 184352514}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp100-winq_fbr.png", "start": 184352514, "end": 184355119}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp100_side_fbr.png", "start": 184355119, "end": 184356013}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp100_top_fbr.png", "start": 184356013, "end": 184356747}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp15_top_fbr.png", "start": 184356747, "end": 184357481}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp15winq_fbr.png", "start": 184357481, "end": 184360267}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp25-winq_fbr.png", "start": 184360267, "end": 184362831}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp25_side_fbr.png", "start": 184362831, "end": 184363657}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp25_top_fbr.png", "start": 184363657, "end": 184364396}, {"filename": "/GameData/textures/lq_health_ammo/plus_1explob2_s1_fbr.png", "start": 184364396, "end": 184365503}, {"filename": "/GameData/textures/lq_health_ammo/plus_1explob_s1_fbr.png", "start": 184365503, "end": 184366136}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp100-winq_fbr.png", "start": 184366136, "end": 184368740}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp100_side_fbr.png", "start": 184368740, "end": 184369634}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp25-winq_fbr.png", "start": 184369634, "end": 184372192}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp25_side_fbr.png", "start": 184372192, "end": 184373017}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp25_top_fbr.png", "start": 184373017, "end": 184373755}, {"filename": "/GameData/textures/lq_health_ammo/plus_2explob2_s1_fbr.png", "start": 184373755, "end": 184374851}, {"filename": "/GameData/textures/lq_health_ammo/plus_2explob_s1_fbr.png", "start": 184374851, "end": 184375493}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp100-winq_fbr.png", "start": 184375493, "end": 184378103}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp100_side_fbr.png", "start": 184378103, "end": 184379000}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp25-winq_fbr.png", "start": 184379000, "end": 184381569}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp25_side_fbr.png", "start": 184381569, "end": 184382400}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp25_top_fbr.png", "start": 184382400, "end": 184383142}, {"filename": "/GameData/textures/lq_health_ammo/plus_3explob2_s1_fbr.png", "start": 184383142, "end": 184384238}, {"filename": "/GameData/textures/lq_health_ammo/plus_3explob_s1_fbr.png", "start": 184384238, "end": 184384880}, {"filename": "/GameData/textures/lq_health_ammo/shells.png", "start": 184384880, "end": 184385826}, {"filename": "/GameData/textures/lq_health_ammo/shellssmall.png", "start": 184385826, "end": 184386552}, {"filename": "/GameData/textures/lq_health_ammo/zap.png", "start": 184386552, "end": 184387545}, {"filename": "/GameData/textures/lq_health_ammo/zapsmall.png", "start": 184387545, "end": 184388309}, {"filename": "/GameData/textures/lq_legacy/brick7.png", "start": 184388309, "end": 184391242}, {"filename": "/GameData/textures/lq_legacy/brick8.png", "start": 184391242, "end": 184394329}, {"filename": "/GameData/textures/lq_legacy/button_0.png", "start": 184394329, "end": 184397801}, {"filename": "/GameData/textures/lq_legacy/button_0_grey.png", "start": 184397801, "end": 184401011}, {"filename": "/GameData/textures/lq_legacy/button_0_grn.png", "start": 184401011, "end": 184404346}, {"filename": "/GameData/textures/lq_legacy/button_1.png", "start": 184404346, "end": 184407801}, {"filename": "/GameData/textures/lq_legacy/button_1_grey.png", "start": 184407801, "end": 184410996}, {"filename": "/GameData/textures/lq_legacy/button_1_grn.png", "start": 184410996, "end": 184414297}, {"filename": "/GameData/textures/lq_legacy/floor_temp.png", "start": 184414297, "end": 184416355}, {"filename": "/GameData/textures/lq_legacy/flr.png", "start": 184416355, "end": 184419112}, {"filename": "/GameData/textures/lq_legacy/gardgrass_1.png", "start": 184419112, "end": 184439282}, {"filename": "/GameData/textures/lq_legacy/go-savgx.png", "start": 184439282, "end": 184441031}, {"filename": "/GameData/textures/lq_legacy/grass.png", "start": 184441031, "end": 184444432}, {"filename": "/GameData/textures/lq_legacy/grk_brk15_c_old.png", "start": 184444432, "end": 184454959}, {"filename": "/GameData/textures/lq_legacy/grk_brk17_f_old.png", "start": 184454959, "end": 184463668}, {"filename": "/GameData/textures/lq_legacy/grk_door1_old.png", "start": 184463668, "end": 184474685}, {"filename": "/GameData/textures/lq_legacy/grk_door2_old.png", "start": 184474685, "end": 184483858}, {"filename": "/GameData/textures/lq_legacy/grk_door3_old.png", "start": 184483858, "end": 184492664}, {"filename": "/GameData/textures/lq_legacy/grk_ebrick22_old.png", "start": 184492664, "end": 184501981}, {"filename": "/GameData/textures/lq_legacy/grk_trim1_7_s_old.png", "start": 184501981, "end": 184504186}, {"filename": "/GameData/textures/lq_legacy/ground_1.png", "start": 184504186, "end": 184506367}, {"filename": "/GameData/textures/lq_legacy/leaves.png", "start": 184506367, "end": 184509429}, {"filename": "/GameData/textures/lq_legacy/marble1_4.png", "start": 184509429, "end": 184512332}, {"filename": "/GameData/textures/lq_legacy/marble1_5.png", "start": 184512332, "end": 184515422}, {"filename": "/GameData/textures/lq_legacy/med_cflat1_3.png", "start": 184515422, "end": 184517903}, {"filename": "/GameData/textures/lq_legacy/med_csl_trm3.png", "start": 184517903, "end": 184519228}, {"filename": "/GameData/textures/lq_legacy/med_csl_trm3b.png", "start": 184519228, "end": 184520218}, {"filename": "/GameData/textures/lq_legacy/med_csl_trm3c.png", "start": 184520218, "end": 184521746}, {"filename": "/GameData/textures/lq_legacy/med_flat10.png", "start": 184521746, "end": 184525639}, {"filename": "/GameData/textures/lq_legacy/med_flat11.png", "start": 184525639, "end": 184528844}, {"filename": "/GameData/textures/lq_legacy/med_flat13.png", "start": 184528844, "end": 184539621}, {"filename": "/GameData/textures/lq_legacy/med_flat14.png", "start": 184539621, "end": 184550022}, {"filename": "/GameData/textures/lq_legacy/med_flat9a.png", "start": 184550022, "end": 184593602}, {"filename": "/GameData/textures/lq_legacy/med_flat9b.png", "start": 184593602, "end": 184635501}, {"filename": "/GameData/textures/lq_legacy/med_plaster1.png", "start": 184635501, "end": 184643543}, {"filename": "/GameData/textures/lq_legacy/med_ret_rock1.png", "start": 184643543, "end": 184647362}, {"filename": "/GameData/textures/lq_legacy/med_ret_wood1_old.png", "start": 184647362, "end": 184654443}, {"filename": "/GameData/textures/lq_legacy/med_rmet_key.png", "start": 184654443, "end": 184658404}, {"filename": "/GameData/textures/lq_legacy/med_rock6.png", "start": 184658404, "end": 184704007}, {"filename": "/GameData/textures/lq_legacy/med_rock7.png", "start": 184704007, "end": 184706891}, {"filename": "/GameData/textures/lq_legacy/med_rock8.png", "start": 184706891, "end": 184708776}, {"filename": "/GameData/textures/lq_legacy/med_rough_block.png", "start": 184708776, "end": 184757619}, {"filename": "/GameData/textures/lq_legacy/med_rough_block_f.png", "start": 184757619, "end": 184812164}, {"filename": "/GameData/textures/lq_legacy/med_tile.png", "start": 184812164, "end": 184814300}, {"filename": "/GameData/textures/lq_legacy/med_wall1.png", "start": 184814300, "end": 184824165}, {"filename": "/GameData/textures/lq_legacy/metground_1.png", "start": 184824165, "end": 184827096}, {"filename": "/GameData/textures/lq_legacy/note-savgx.png", "start": 184827096, "end": 184846116}, {"filename": "/GameData/textures/lq_legacy/plus_0button1.png", "start": 184846116, "end": 184849561}, {"filename": "/GameData/textures/lq_legacy/plus_0button2_fbr.png", "start": 184849561, "end": 184851048}, {"filename": "/GameData/textures/lq_legacy/plus_1button2_fbr.png", "start": 184851048, "end": 184852536}, {"filename": "/GameData/textures/lq_legacy/plus_1button3.png", "start": 184852536, "end": 184854416}, {"filename": "/GameData/textures/lq_legacy/plus_abutton1_fbr.png", "start": 184854416, "end": 184858018}, {"filename": "/GameData/textures/lq_legacy/plus_abutton2_fbr.png", "start": 184858018, "end": 184859516}, {"filename": "/GameData/textures/lq_legacy/readme.txt", "start": 184859516, "end": 184859593}, {"filename": "/GameData/textures/lq_legacy/riktoiflat.png", "start": 184859593, "end": 184873712}, {"filename": "/GameData/textures/lq_legacy/riktoiflat_blu.png", "start": 184873712, "end": 184886647}, {"filename": "/GameData/textures/lq_legacy/riktoiflat_grn.png", "start": 184886647, "end": 184898301}, {"filename": "/GameData/textures/lq_legacy/riktoilava.png", "start": 184898301, "end": 184902805}, {"filename": "/GameData/textures/lq_legacy/riktoislime.png", "start": 184902805, "end": 184906510}, {"filename": "/GameData/textures/lq_legacy/riktoitrim.png", "start": 184906510, "end": 184907997}, {"filename": "/GameData/textures/lq_legacy/riktoitrim__purp.png", "start": 184907997, "end": 184909713}, {"filename": "/GameData/textures/lq_legacy/riktoitrim_blu.png", "start": 184909713, "end": 184911443}, {"filename": "/GameData/textures/lq_legacy/riktoiwall.png", "start": 184911443, "end": 184951240}, {"filename": "/GameData/textures/lq_legacy/riktoiwall__purp.png", "start": 184951240, "end": 184987313}, {"filename": "/GameData/textures/lq_legacy/riktoiwall_blu.png", "start": 184987313, "end": 185023374}, {"filename": "/GameData/textures/lq_legacy/riktoiwater.png", "start": 185023374, "end": 185026265}, {"filename": "/GameData/textures/lq_legacy/rune1_fbr.png", "start": 185026265, "end": 185029278}, {"filename": "/GameData/textures/lq_legacy/rune2_fbr.png", "start": 185029278, "end": 185032344}, {"filename": "/GameData/textures/lq_legacy/rune3_fbr.png", "start": 185032344, "end": 185035740}, {"filename": "/GameData/textures/lq_legacy/rune4_fbr.png", "start": 185035740, "end": 185038292}, {"filename": "/GameData/textures/lq_legacy/sign_easy.png", "start": 185038292, "end": 185043244}, {"filename": "/GameData/textures/lq_legacy/sign_empty.png", "start": 185043244, "end": 185048001}, {"filename": "/GameData/textures/lq_legacy/sign_hard.png", "start": 185048001, "end": 185052874}, {"filename": "/GameData/textures/lq_legacy/sign_medium.png", "start": 185052874, "end": 185057884}, {"filename": "/GameData/textures/lq_legacy/sign_metal_1.png", "start": 185057884, "end": 185059096}, {"filename": "/GameData/textures/lq_legacy/sign_metal_2.png", "start": 185059096, "end": 185060691}, {"filename": "/GameData/textures/lq_legacy/sign_nmare.png", "start": 185060691, "end": 185065712}, {"filename": "/GameData/textures/lq_legacy/sky2.png", "start": 185065712, "end": 185081202}, {"filename": "/GameData/textures/lq_legacy/sky4.png", "start": 185081202, "end": 185090714}, {"filename": "/GameData/textures/lq_legacy/sky5_fbr.png", "start": 185090714, "end": 185107867}, {"filename": "/GameData/textures/lq_legacy/sky5a.png", "start": 185107867, "end": 185116878}, {"filename": "/GameData/textures/lq_legacy/sky7.png", "start": 185116878, "end": 185130793}, {"filename": "/GameData/textures/lq_legacy/sky8.png", "start": 185130793, "end": 185147466}, {"filename": "/GameData/textures/lq_legacy/sky8a_fbr.png", "start": 185147466, "end": 185156262}, {"filename": "/GameData/textures/lq_legacy/tile.png", "start": 185156262, "end": 185165078}, {"filename": "/GameData/textures/lq_legacy/tile_blu.png", "start": 185165078, "end": 185174084}, {"filename": "/GameData/textures/lq_legacy/tile_grn.png", "start": 185174084, "end": 185182347}, {"filename": "/GameData/textures/lq_legacy/trim2_blu.png", "start": 185182347, "end": 185183460}, {"filename": "/GameData/textures/lq_legacy/trim2_grn.png", "start": 185183460, "end": 185184524}, {"filename": "/GameData/textures/lq_legacy/ultrasteel1.png", "start": 185184524, "end": 185197414}, {"filename": "/GameData/textures/lq_legacy/ultrasteel2.png", "start": 185197414, "end": 185209208}, {"filename": "/GameData/textures/lq_legacy/vines1_old.png", "start": 185209208, "end": 185214062}, {"filename": "/GameData/textures/lq_legacy/wiz1_4.png", "start": 185214062, "end": 185217883}, {"filename": "/GameData/textures/lq_liquidsky/+0water_f3.png", "start": 185217883, "end": 185220824}, {"filename": "/GameData/textures/lq_liquidsky/+1water_f3.png", "start": 185220824, "end": 185223748}, {"filename": "/GameData/textures/lq_liquidsky/+2water_f3.png", "start": 185223748, "end": 185226666}, {"filename": "/GameData/textures/lq_liquidsky/+3water_f3.png", "start": 185226666, "end": 185229518}, {"filename": "/GameData/textures/lq_liquidsky/plus_0blood_f1.png", "start": 185229518, "end": 185231307}, {"filename": "/GameData/textures/lq_liquidsky/plus_0fslime.png", "start": 185231307, "end": 185245263}, {"filename": "/GameData/textures/lq_liquidsky/plus_0lava_fall3_fbr.png", "start": 185245263, "end": 185254256}, {"filename": "/GameData/textures/lq_liquidsky/plus_0water_f1.png", "start": 185254256, "end": 185256105}, {"filename": "/GameData/textures/lq_liquidsky/plus_0water_f2.png", "start": 185256105, "end": 185257923}, {"filename": "/GameData/textures/lq_liquidsky/plus_0wfall0.png", "start": 185257923, "end": 185267532}, {"filename": "/GameData/textures/lq_liquidsky/plus_1blood_f1.png", "start": 185267532, "end": 185269337}, {"filename": "/GameData/textures/lq_liquidsky/plus_1fslime.png", "start": 185269337, "end": 185283080}, {"filename": "/GameData/textures/lq_liquidsky/plus_1lava_fall3_fbr.png", "start": 185283080, "end": 185292647}, {"filename": "/GameData/textures/lq_liquidsky/plus_1water_f1.png", "start": 185292647, "end": 185294295}, {"filename": "/GameData/textures/lq_liquidsky/plus_1water_f2.png", "start": 185294295, "end": 185296124}, {"filename": "/GameData/textures/lq_liquidsky/plus_1wfall0.png", "start": 185296124, "end": 185305775}, {"filename": "/GameData/textures/lq_liquidsky/plus_2blood_f1.png", "start": 185305775, "end": 185307633}, {"filename": "/GameData/textures/lq_liquidsky/plus_2fslime.png", "start": 185307633, "end": 185321527}, {"filename": "/GameData/textures/lq_liquidsky/plus_2lava_fall3_fbr.png", "start": 185321527, "end": 185331076}, {"filename": "/GameData/textures/lq_liquidsky/plus_2water_f1.png", "start": 185331076, "end": 185332847}, {"filename": "/GameData/textures/lq_liquidsky/plus_2water_f2.png", "start": 185332847, "end": 185334733}, {"filename": "/GameData/textures/lq_liquidsky/plus_2wfall0.png", "start": 185334733, "end": 185344395}, {"filename": "/GameData/textures/lq_liquidsky/plus_3blood_f1.png", "start": 185344395, "end": 185346195}, {"filename": "/GameData/textures/lq_liquidsky/plus_3fslime.png", "start": 185346195, "end": 185359878}, {"filename": "/GameData/textures/lq_liquidsky/plus_3lava_fall3_fbr.png", "start": 185359878, "end": 185369215}, {"filename": "/GameData/textures/lq_liquidsky/plus_3water_f1.png", "start": 185369215, "end": 185370866}, {"filename": "/GameData/textures/lq_liquidsky/plus_3water_f2.png", "start": 185370866, "end": 185372693}, {"filename": "/GameData/textures/lq_liquidsky/plus_3wfall0.png", "start": 185372693, "end": 185382396}, {"filename": "/GameData/textures/lq_liquidsky/plus_4fslime.png", "start": 185382396, "end": 185396060}, {"filename": "/GameData/textures/lq_liquidsky/plus_4lava_fall3_fbr.png", "start": 185396060, "end": 185405433}, {"filename": "/GameData/textures/lq_liquidsky/plus_4wfall0.png", "start": 185405433, "end": 185415012}, {"filename": "/GameData/textures/lq_liquidsky/plus_5fslime.png", "start": 185415012, "end": 185428780}, {"filename": "/GameData/textures/lq_liquidsky/plus_5lava_fall3_fbr.png", "start": 185428780, "end": 185438143}, {"filename": "/GameData/textures/lq_liquidsky/plus_5wfall0.png", "start": 185438143, "end": 185447742}, {"filename": "/GameData/textures/lq_liquidsky/plus_6fslime.png", "start": 185447742, "end": 185461594}, {"filename": "/GameData/textures/lq_liquidsky/plus_6lava_fall3_fbr.png", "start": 185461594, "end": 185470979}, {"filename": "/GameData/textures/lq_liquidsky/plus_6wfall0.png", "start": 185470979, "end": 185480564}, {"filename": "/GameData/textures/lq_liquidsky/plus_7fslime.png", "start": 185480564, "end": 185494415}, {"filename": "/GameData/textures/lq_liquidsky/plus_7lava_fall3_fbr.png", "start": 185494415, "end": 185503930}, {"filename": "/GameData/textures/lq_liquidsky/plus_7wfall0.png", "start": 185503930, "end": 185513599}, {"filename": "/GameData/textures/lq_liquidsky/plus_8wfall0.png", "start": 185513599, "end": 185523261}, {"filename": "/GameData/textures/lq_liquidsky/plus_9wfall0.png", "start": 185523261, "end": 185532883}, {"filename": "/GameData/textures/lq_liquidsky/sky-test.png", "start": 185532883, "end": 185550479}, {"filename": "/GameData/textures/lq_liquidsky/sky-test.xcf", "start": 185550479, "end": 185683744}, {"filename": "/GameData/textures/lq_liquidsky/sky5_blu.png", "start": 185683744, "end": 185697057}, {"filename": "/GameData/textures/lq_liquidsky/sky5_dismal.png", "start": 185697057, "end": 185710240}, {"filename": "/GameData/textures/lq_liquidsky/sky_galx_fbr.png", "start": 185710240, "end": 185732995}, {"filename": "/GameData/textures/lq_liquidsky/sky_galx_spark_fbr.png", "start": 185732995, "end": 185752553}, {"filename": "/GameData/textures/lq_liquidsky/sky_orng.png", "start": 185752553, "end": 185767192}, {"filename": "/GameData/textures/lq_liquidsky/sky_pando.png", "start": 185767192, "end": 185783300}, {"filename": "/GameData/textures/lq_liquidsky/sky_pando2.png", "start": 185783300, "end": 185799835}, {"filename": "/GameData/textures/lq_liquidsky/sky_star.png", "start": 185799835, "end": 185801495}, {"filename": "/GameData/textures/lq_liquidsky/sky_void.png", "start": 185801495, "end": 185802190}, {"filename": "/GameData/textures/lq_liquidsky/sky_wfog_fbr.png", "start": 185802190, "end": 185803096}, {"filename": "/GameData/textures/lq_liquidsky/star_acid.png", "start": 185803096, "end": 185805275}, {"filename": "/GameData/textures/lq_liquidsky/star_blood1.png", "start": 185805275, "end": 185807348}, {"filename": "/GameData/textures/lq_liquidsky/star_lava1_fbr.png", "start": 185807348, "end": 185810553}, {"filename": "/GameData/textures/lq_liquidsky/star_lava2_fbr.png", "start": 185810553, "end": 185814635}, {"filename": "/GameData/textures/lq_liquidsky/star_lava3_fbr.png", "start": 185814635, "end": 185818604}, {"filename": "/GameData/textures/lq_liquidsky/star_lava_void_fbr.png", "start": 185818604, "end": 185822294}, {"filename": "/GameData/textures/lq_liquidsky/star_lavaskip.png", "start": 185822294, "end": 185823407}, {"filename": "/GameData/textures/lq_liquidsky/star_meatgoo2_fbr.png", "start": 185823407, "end": 185826595}, {"filename": "/GameData/textures/lq_liquidsky/star_meatgoo_fbr.png", "start": 185826595, "end": 185829392}, {"filename": "/GameData/textures/lq_liquidsky/star_slime1.png", "start": 185829392, "end": 185835683}, {"filename": "/GameData/textures/lq_liquidsky/star_slime2.png", "start": 185835683, "end": 185838872}, {"filename": "/GameData/textures/lq_liquidsky/star_slime3.png", "start": 185838872, "end": 185840904}, {"filename": "/GameData/textures/lq_liquidsky/star_slime_soul.png", "start": 185840904, "end": 185843694}, {"filename": "/GameData/textures/lq_liquidsky/star_slimeskip.png", "start": 185843694, "end": 185844767}, {"filename": "/GameData/textures/lq_liquidsky/star_soul_drain.png", "start": 185844767, "end": 185847735}, {"filename": "/GameData/textures/lq_liquidsky/star_tele1_fbr.png", "start": 185847735, "end": 185849615}, {"filename": "/GameData/textures/lq_liquidsky/star_tele2_fbr.png", "start": 185849615, "end": 185852416}, {"filename": "/GameData/textures/lq_liquidsky/star_tele3_fbr.png", "start": 185852416, "end": 185855452}, {"filename": "/GameData/textures/lq_liquidsky/star_tele4_fbr.png", "start": 185855452, "end": 185858144}, {"filename": "/GameData/textures/lq_liquidsky/star_water0.png", "start": 185858144, "end": 185860992}, {"filename": "/GameData/textures/lq_liquidsky/star_water1.png", "start": 185860992, "end": 185864101}, {"filename": "/GameData/textures/lq_liquidsky/star_water2.png", "start": 185864101, "end": 185866361}, {"filename": "/GameData/textures/lq_liquidsky/star_water3.png", "start": 185866361, "end": 185868742}, {"filename": "/GameData/textures/lq_liquidsky/star_water4.png", "start": 185868742, "end": 185873529}, {"filename": "/GameData/textures/lq_liquidsky/star_waterskip.png", "start": 185873529, "end": 185875269}, {"filename": "/GameData/textures/lq_liquidsky/star_wstill0.png", "start": 185875269, "end": 185878108}, {"filename": "/GameData/textures/lq_mayan/btn1.png", "start": 185878108, "end": 185881026}, {"filename": "/GameData/textures/lq_mayan/may_arrow.png", "start": 185881026, "end": 185881907}, {"filename": "/GameData/textures/lq_mayan/may_blok1_1.png", "start": 185881907, "end": 185884428}, {"filename": "/GameData/textures/lq_mayan/may_blok1_2.png", "start": 185884428, "end": 185886619}, {"filename": "/GameData/textures/lq_mayan/may_blok1_2_m.png", "start": 185886619, "end": 185889715}, {"filename": "/GameData/textures/lq_mayan/may_blok1_m.png", "start": 185889715, "end": 185898989}, {"filename": "/GameData/textures/lq_mayan/may_blok2_1.png", "start": 185898989, "end": 185901553}, {"filename": "/GameData/textures/lq_mayan/may_blok2_1_m.png", "start": 185901553, "end": 185904774}, {"filename": "/GameData/textures/lq_mayan/may_blok2_2.png", "start": 185904774, "end": 185915037}, {"filename": "/GameData/textures/lq_mayan/may_blok2_2_m.png", "start": 185915037, "end": 185927465}, {"filename": "/GameData/textures/lq_mayan/may_blud1_1.png", "start": 185927465, "end": 185930057}, {"filename": "/GameData/textures/lq_mayan/may_blud1_1m.png", "start": 185930057, "end": 185933277}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_1.png", "start": 185933277, "end": 185935712}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_1m.png", "start": 185935712, "end": 185938782}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_2.png", "start": 185938782, "end": 185941209}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_2m.png", "start": 185941209, "end": 185944237}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_3.png", "start": 185944237, "end": 185946496}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_3m.png", "start": 185946496, "end": 185949905}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_5.png", "start": 185949905, "end": 185952758}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_5m.png", "start": 185952758, "end": 185956407}, {"filename": "/GameData/textures/lq_mayan/may_bnd_skull.png", "start": 185956407, "end": 185959039}, {"filename": "/GameData/textures/lq_mayan/may_brk1_0.png", "start": 185959039, "end": 185962143}, {"filename": "/GameData/textures/lq_mayan/may_brk1_0m.png", "start": 185962143, "end": 185965200}, {"filename": "/GameData/textures/lq_mayan/may_brk1_1.png", "start": 185965200, "end": 185967882}, {"filename": "/GameData/textures/lq_mayan/may_brk1_1m.png", "start": 185967882, "end": 185971081}, {"filename": "/GameData/textures/lq_mayan/may_brk1_2.png", "start": 185971081, "end": 185974032}, {"filename": "/GameData/textures/lq_mayan/may_brk1_2m.png", "start": 185974032, "end": 185977437}, {"filename": "/GameData/textures/lq_mayan/may_brk1_3.png", "start": 185977437, "end": 185980374}, {"filename": "/GameData/textures/lq_mayan/may_brk1_3m.png", "start": 185980374, "end": 185983550}, {"filename": "/GameData/textures/lq_mayan/may_brk2_0.png", "start": 185983550, "end": 185993405}, {"filename": "/GameData/textures/lq_mayan/may_brk2_0_m.png", "start": 185993405, "end": 186005738}, {"filename": "/GameData/textures/lq_mayan/may_brk_old.png", "start": 186005738, "end": 186008337}, {"filename": "/GameData/textures/lq_mayan/may_brk_oldm.png", "start": 186008337, "end": 186011676}, {"filename": "/GameData/textures/lq_mayan/may_deco1_1.png", "start": 186011676, "end": 186022905}, {"filename": "/GameData/textures/lq_mayan/may_deco1_1m.png", "start": 186022905, "end": 186036940}, {"filename": "/GameData/textures/lq_mayan/may_deco1_2.png", "start": 186036940, "end": 186039460}, {"filename": "/GameData/textures/lq_mayan/may_deco1_2my.png", "start": 186039460, "end": 186042848}, {"filename": "/GameData/textures/lq_mayan/may_deco1_3.png", "start": 186042848, "end": 186061421}, {"filename": "/GameData/textures/lq_mayan/may_deco1_3m.png", "start": 186061421, "end": 186081772}, {"filename": "/GameData/textures/lq_mayan/may_door1_1.png", "start": 186081772, "end": 186091239}, {"filename": "/GameData/textures/lq_mayan/may_door1_1m.png", "start": 186091239, "end": 186102330}, {"filename": "/GameData/textures/lq_mayan/may_door2_1.png", "start": 186102330, "end": 186114298}, {"filename": "/GameData/textures/lq_mayan/may_door2_2.png", "start": 186114298, "end": 186126076}, {"filename": "/GameData/textures/lq_mayan/may_drt2_1.png", "start": 186126076, "end": 186128913}, {"filename": "/GameData/textures/lq_mayan/may_flr1_1.png", "start": 186128913, "end": 186131818}, {"filename": "/GameData/textures/lq_mayan/may_flr1_2.png", "start": 186131818, "end": 186135041}, {"filename": "/GameData/textures/lq_mayan/may_flt1_1.png", "start": 186135041, "end": 186137330}, {"filename": "/GameData/textures/lq_mayan/may_flt1_1m.png", "start": 186137330, "end": 186140778}, {"filename": "/GameData/textures/lq_mayan/may_key1_1.png", "start": 186140778, "end": 186142365}, {"filename": "/GameData/textures/lq_mayan/may_key1_2.png", "start": 186142365, "end": 186144362}, {"filename": "/GameData/textures/lq_mayan/may_lite1_1_fbr.png", "start": 186144362, "end": 186145292}, {"filename": "/GameData/textures/lq_mayan/may_lite1_2.png", "start": 186145292, "end": 186145655}, {"filename": "/GameData/textures/lq_mayan/may_lite2_1.png", "start": 186145655, "end": 186148592}, {"filename": "/GameData/textures/lq_mayan/may_lite2_2.png", "start": 186148592, "end": 186149078}, {"filename": "/GameData/textures/lq_mayan/may_lite3_1_fbr.png", "start": 186149078, "end": 186150035}, {"filename": "/GameData/textures/lq_mayan/may_lite_f1.png", "start": 186150035, "end": 186150521}, {"filename": "/GameData/textures/lq_mayan/may_oldmtomb1_1_fbr.png", "start": 186150521, "end": 186164373}, {"filename": "/GameData/textures/lq_mayan/may_oldtomb1_2_fbr.png", "start": 186164373, "end": 186178265}, {"filename": "/GameData/textures/lq_mayan/may_plat_stem_m.png", "start": 186178265, "end": 186179131}, {"filename": "/GameData/textures/lq_mayan/may_plats.png", "start": 186179131, "end": 186181793}, {"filename": "/GameData/textures/lq_mayan/may_platst.png", "start": 186181793, "end": 186184220}, {"filename": "/GameData/textures/lq_mayan/may_platt.png", "start": 186184220, "end": 186186843}, {"filename": "/GameData/textures/lq_mayan/may_plr1_1.png", "start": 186186843, "end": 186190037}, {"filename": "/GameData/textures/lq_mayan/may_tomb1_1_fbr.png", "start": 186190037, "end": 186203889}, {"filename": "/GameData/textures/lq_mayan/may_tomb1_2_fbr.png", "start": 186203889, "end": 186217781}, {"filename": "/GameData/textures/lq_mayan/may_trm1_1.png", "start": 186217781, "end": 186226709}, {"filename": "/GameData/textures/lq_mayan/may_trm1_2.png", "start": 186226709, "end": 186229414}, {"filename": "/GameData/textures/lq_mayan/may_trm1_a.png", "start": 186229414, "end": 186231756}, {"filename": "/GameData/textures/lq_mayan/may_tskull.png", "start": 186231756, "end": 186242762}, {"filename": "/GameData/textures/lq_mayan/may_wall1_1.png", "start": 186242762, "end": 186245744}, {"filename": "/GameData/textures/lq_mayan/may_wall1_2.png", "start": 186245744, "end": 186248710}, {"filename": "/GameData/textures/lq_mayan/may_wall1_3.png", "start": 186248710, "end": 186251904}, {"filename": "/GameData/textures/lq_mayan/may_wall1_3a.png", "start": 186251904, "end": 186254485}, {"filename": "/GameData/textures/lq_mayan/may_wall1_4.png", "start": 186254485, "end": 186257473}, {"filename": "/GameData/textures/lq_mayan/may_wall1_4a.png", "start": 186257473, "end": 186259872}, {"filename": "/GameData/textures/lq_mayan/maya_end_dr1.png", "start": 186259872, "end": 186264258}, {"filename": "/GameData/textures/lq_mayan/maya_end_dr2.png", "start": 186264258, "end": 186268643}, {"filename": "/GameData/textures/lq_mayan/maya_end_trim1.png", "start": 186268643, "end": 186273033}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_btn1.png", "start": 186273033, "end": 186275950}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mpiloilon_fbr.png", "start": 186275950, "end": 186277508}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mpilon_fbr.png", "start": 186277508, "end": 186279083}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mshoohoot_fbr.png", "start": 186279083, "end": 186280028}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mshoot_fbr.png", "start": 186280028, "end": 186280971}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_btn1.png", "start": 186280971, "end": 186283879}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mpiloilon_fbr.png", "start": 186283879, "end": 186285452}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mpilon_fbr.png", "start": 186285452, "end": 186287045}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mshoohoot_fbr.png", "start": 186287045, "end": 186288030}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mshoot_fbr.png", "start": 186288030, "end": 186289014}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_btn1.png", "start": 186289014, "end": 186291905}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_mpilon.png", "start": 186291905, "end": 186293482}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_mshoohoot_fbr.png", "start": 186293482, "end": 186294507}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_mshoot_fbr.png", "start": 186294507, "end": 186295534}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_btn1.png", "start": 186295534, "end": 186298596}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mpiloilon_fbr.png", "start": 186298596, "end": 186300169}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mpilon_fbr.png", "start": 186300169, "end": 186301762}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mshoohoot_fbr.png", "start": 186301762, "end": 186302747}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mshoot_fbr.png", "start": 186302747, "end": 186303731}, {"filename": "/GameData/textures/lq_mayan/plus_4_may_btn1.png", "start": 186303731, "end": 186306794}, {"filename": "/GameData/textures/lq_mayan/plus_5_may_btn1.png", "start": 186306794, "end": 186309535}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_btn1.png", "start": 186309535, "end": 186312258}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_mpiloilon_fbr.png", "start": 186312258, "end": 186313835}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_mpilon_fbr.png", "start": 186313835, "end": 186315419}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_mshoot.png", "start": 186315419, "end": 186316368}, {"filename": "/GameData/textures/lq_medieval/+0med_but1.png", "start": 186316368, "end": 186320623}, {"filename": "/GameData/textures/lq_medieval/+0med_but2.png", "start": 186320623, "end": 186322335}, {"filename": "/GameData/textures/lq_medieval/+0med_but3.png", "start": 186322335, "end": 186324038}, {"filename": "/GameData/textures/lq_medieval/+0med_but_s1.png", "start": 186324038, "end": 186328284}, {"filename": "/GameData/textures/lq_medieval/+0med_sht_but1.png", "start": 186328284, "end": 186329577}, {"filename": "/GameData/textures/lq_medieval/+1med_but3.png", "start": 186329577, "end": 186331279}, {"filename": "/GameData/textures/lq_medieval/+1med_but_s1.png", "start": 186331279, "end": 186335498}, {"filename": "/GameData/textures/lq_medieval/+1med_sht_but1.png", "start": 186335498, "end": 186336811}, {"filename": "/GameData/textures/lq_medieval/+2med_but_s1.png", "start": 186336811, "end": 186341085}, {"filename": "/GameData/textures/lq_medieval/+3med_but_s1.png", "start": 186341085, "end": 186345304}, {"filename": "/GameData/textures/lq_medieval/+amed_but1.png", "start": 186345304, "end": 186349387}, {"filename": "/GameData/textures/lq_medieval/+amed_but2.png", "start": 186349387, "end": 186351105}, {"filename": "/GameData/textures/lq_medieval/+amed_but3.png", "start": 186351105, "end": 186352715}, {"filename": "/GameData/textures/lq_medieval/+amed_but_s1.png", "start": 186352715, "end": 186356928}, {"filename": "/GameData/textures/lq_medieval/+amed_sht_but1.png", "start": 186356928, "end": 186358084}, {"filename": "/GameData/textures/lq_medieval/Art1.png", "start": 186358084, "end": 186491077}, {"filename": "/GameData/textures/lq_medieval/afloor1_4.png", "start": 186491077, "end": 186494078}, {"filename": "/GameData/textures/lq_medieval/afloor1_8.png", "start": 186494078, "end": 186496918}, {"filename": "/GameData/textures/lq_medieval/afloor3_1.png", "start": 186496918, "end": 186499728}, {"filename": "/GameData/textures/lq_medieval/altar1_1.png", "start": 186499728, "end": 186503616}, {"filename": "/GameData/textures/lq_medieval/altar1_3.png", "start": 186503616, "end": 186508097}, {"filename": "/GameData/textures/lq_medieval/altar1_4.png", "start": 186508097, "end": 186512150}, {"filename": "/GameData/textures/lq_medieval/brick0.png", "start": 186512150, "end": 186520372}, {"filename": "/GameData/textures/lq_medieval/brick1.png", "start": 186520372, "end": 186531126}, {"filename": "/GameData/textures/lq_medieval/brick4_s.png", "start": 186531126, "end": 186533059}, {"filename": "/GameData/textures/lq_medieval/brown1.png", "start": 186533059, "end": 186542235}, {"filename": "/GameData/textures/lq_medieval/med_block_1a.png", "start": 186542235, "end": 186588590}, {"filename": "/GameData/textures/lq_medieval/med_block_1b.png", "start": 186588590, "end": 186671954}, {"filename": "/GameData/textures/lq_medieval/med_block_1c.png", "start": 186671954, "end": 186770125}, {"filename": "/GameData/textures/lq_medieval/med_block_1d.png", "start": 186770125, "end": 186878794}, {"filename": "/GameData/textures/lq_medieval/med_block_1e.png", "start": 186878794, "end": 186987287}, {"filename": "/GameData/textures/lq_medieval/med_block_1f.png", "start": 186987287, "end": 187055146}, {"filename": "/GameData/textures/lq_medieval/med_block_1s.png", "start": 187055146, "end": 187172597}, {"filename": "/GameData/textures/lq_medieval/med_block_2a.png", "start": 187172597, "end": 187249981}, {"filename": "/GameData/textures/lq_medieval/med_block_2b.png", "start": 187249981, "end": 187327420}, {"filename": "/GameData/textures/lq_medieval/med_block_2c.png", "start": 187327420, "end": 187416295}, {"filename": "/GameData/textures/lq_medieval/med_block_2d.png", "start": 187416295, "end": 187514489}, {"filename": "/GameData/textures/lq_medieval/med_block_2e.png", "start": 187514489, "end": 187612726}, {"filename": "/GameData/textures/lq_medieval/med_block_2f.png", "start": 187612726, "end": 187677270}, {"filename": "/GameData/textures/lq_medieval/med_block_2s.png", "start": 187677270, "end": 187783022}, {"filename": "/GameData/textures/lq_medieval/med_brk9_ceil1a.png", "start": 187783022, "end": 187794830}, {"filename": "/GameData/textures/lq_medieval/med_brk9_ceil1b.png", "start": 187794830, "end": 187807365}, {"filename": "/GameData/textures/lq_medieval/med_brk9_wal2a.png", "start": 187807365, "end": 187820262}, {"filename": "/GameData/textures/lq_medieval/med_brk9_wal2b.png", "start": 187820262, "end": 187833518}, {"filename": "/GameData/textures/lq_medieval/med_brk9_wal2c.png", "start": 187833518, "end": 187845960}, {"filename": "/GameData/textures/lq_medieval/med_brk9_win1.png", "start": 187845960, "end": 187888958}, {"filename": "/GameData/textures/lq_medieval/med_brk9_win1b.png", "start": 187888958, "end": 187934284}, {"filename": "/GameData/textures/lq_medieval/med_but_side.png", "start": 187934284, "end": 187935282}, {"filename": "/GameData/textures/lq_medieval/med_cmet1.png", "start": 187935282, "end": 187938406}, {"filename": "/GameData/textures/lq_medieval/med_cmet2a.png", "start": 187938406, "end": 187941557}, {"filename": "/GameData/textures/lq_medieval/med_cmet2b.png", "start": 187941557, "end": 187944619}, {"filename": "/GameData/textures/lq_medieval/med_cmet2c.png", "start": 187944619, "end": 187947756}, {"filename": "/GameData/textures/lq_medieval/med_cmet3a.png", "start": 187947756, "end": 187951028}, {"filename": "/GameData/textures/lq_medieval/med_cmet3b.png", "start": 187951028, "end": 187954210}, {"filename": "/GameData/textures/lq_medieval/med_cmet4.png", "start": 187954210, "end": 187957051}, {"filename": "/GameData/textures/lq_medieval/med_cmet5a.png", "start": 187957051, "end": 187960189}, {"filename": "/GameData/textures/lq_medieval/med_cmet5c.png", "start": 187960189, "end": 187963332}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk10.png", "start": 187963332, "end": 187978058}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk10_f.png", "start": 187978058, "end": 187990346}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk10b.png", "start": 187990346, "end": 188004909}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk11.png", "start": 188004909, "end": 188016157}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk12.png", "start": 188016157, "end": 188033106}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk12_f.png", "start": 188033106, "end": 188050439}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk13.png", "start": 188050439, "end": 188106556}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk14.png", "start": 188106556, "end": 188122410}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk14_f.png", "start": 188122410, "end": 188135379}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk14b.png", "start": 188135379, "end": 188148869}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk15.png", "start": 188148869, "end": 188162695}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk15b.png", "start": 188162695, "end": 188176625}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk15f.png", "start": 188176625, "end": 188189823}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk16.png", "start": 188189823, "end": 188206044}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk16b.png", "start": 188206044, "end": 188223636}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk16f.png", "start": 188223636, "end": 188240907}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk17.png", "start": 188240907, "end": 188251014}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk17_f.png", "start": 188251014, "end": 188260039}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk17b.png", "start": 188260039, "end": 188270176}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_f.png", "start": 188270176, "end": 188278809}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_gb.png", "start": 188278809, "end": 188281557}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_gt.png", "start": 188281557, "end": 188284294}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_t.png", "start": 188284294, "end": 188292480}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_tb.png", "start": 188292480, "end": 188303106}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_tc.png", "start": 188303106, "end": 188307635}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18b.png", "start": 188307635, "end": 188315808}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk19_f.png", "start": 188315808, "end": 188327509}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk19_t.png", "start": 188327509, "end": 188338900}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk19b.png", "start": 188338900, "end": 188350220}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk1_1.png", "start": 188350220, "end": 188352705}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk1_2.png", "start": 188352705, "end": 188355876}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk1_3.png", "start": 188355876, "end": 188358119}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk2_1.png", "start": 188358119, "end": 188360668}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk2_2.png", "start": 188360668, "end": 188364041}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk5.png", "start": 188364041, "end": 188367208}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk6_1.png", "start": 188367208, "end": 188369851}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk6_2.png", "start": 188369851, "end": 188374913}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk7_1.png", "start": 188374913, "end": 188377553}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk7_1b.png", "start": 188377553, "end": 188380097}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk7_2.png", "start": 188380097, "end": 188383263}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk8_1c.png", "start": 188383263, "end": 188395868}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk8_1d.png", "start": 188395868, "end": 188408471}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk9_1.png", "start": 188408471, "end": 188421681}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk9_1b.png", "start": 188421681, "end": 188435338}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk9_f.png", "start": 188435338, "end": 188448125}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr1_1.png", "start": 188448125, "end": 188458072}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr2_1.png", "start": 188458072, "end": 188460358}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr2_2.png", "start": 188460358, "end": 188462531}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_1.png", "start": 188462531, "end": 188466271}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_3.png", "start": 188466271, "end": 188469705}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_4.png", "start": 188469705, "end": 188482761}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_5.png", "start": 188482761, "end": 188497279}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr5_1.png", "start": 188497279, "end": 188512209}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr5_2.png", "start": 188512209, "end": 188527737}, {"filename": "/GameData/textures/lq_medieval/med_csl_stp1.png", "start": 188527737, "end": 188530334}, {"filename": "/GameData/textures/lq_medieval/med_csl_stp2.png", "start": 188530334, "end": 188532877}, {"filename": "/GameData/textures/lq_medieval/med_csl_trm1.png", "start": 188532877, "end": 188534202}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1.png", "start": 188534202, "end": 188555675}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t.png", "start": 188555675, "end": 188567062}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t2.png", "start": 188567062, "end": 188580224}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t2b_fbr.png", "start": 188580224, "end": 188594174}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t3.png", "start": 188594174, "end": 188605914}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t4.png", "start": 188605914, "end": 188618690}, {"filename": "/GameData/textures/lq_medieval/med_dbrick2.png", "start": 188618690, "end": 188624224}, {"filename": "/GameData/textures/lq_medieval/med_dbrick3.png", "start": 188624224, "end": 188629598}, {"filename": "/GameData/textures/lq_medieval/med_dbrick4.png", "start": 188629598, "end": 188634906}, {"filename": "/GameData/textures/lq_medieval/med_dbrick5.png", "start": 188634906, "end": 188640728}, {"filename": "/GameData/textures/lq_medieval/med_dbrick6.png", "start": 188640728, "end": 188649889}, {"filename": "/GameData/textures/lq_medieval/med_dbrick6b.png", "start": 188649889, "end": 188658897}, {"filename": "/GameData/textures/lq_medieval/med_dbrick6f.png", "start": 188658897, "end": 188668187}, {"filename": "/GameData/textures/lq_medieval/med_door1.png", "start": 188668187, "end": 188672870}, {"filename": "/GameData/textures/lq_medieval/med_door2.png", "start": 188672870, "end": 188677369}, {"filename": "/GameData/textures/lq_medieval/med_door3.png", "start": 188677369, "end": 188682018}, {"filename": "/GameData/textures/lq_medieval/med_door3b.png", "start": 188682018, "end": 188684559}, {"filename": "/GameData/textures/lq_medieval/med_door4.png", "start": 188684559, "end": 188689123}, {"filename": "/GameData/textures/lq_medieval/med_door4b.png", "start": 188689123, "end": 188691719}, {"filename": "/GameData/textures/lq_medieval/med_dr1a.png", "start": 188691719, "end": 188710503}, {"filename": "/GameData/textures/lq_medieval/med_dr1a_blu.png", "start": 188710503, "end": 188728867}, {"filename": "/GameData/textures/lq_medieval/med_dr1b.png", "start": 188728867, "end": 188753016}, {"filename": "/GameData/textures/lq_medieval/med_dr1b_blu.png", "start": 188753016, "end": 188772635}, {"filename": "/GameData/textures/lq_medieval/med_dr2a.png", "start": 188772635, "end": 188797012}, {"filename": "/GameData/textures/lq_medieval/med_dr2a_blu.png", "start": 188797012, "end": 188816906}, {"filename": "/GameData/textures/lq_medieval/med_dr3a.png", "start": 188816906, "end": 188830389}, {"filename": "/GameData/textures/lq_medieval/med_dr3a_blu.png", "start": 188830389, "end": 188843679}, {"filename": "/GameData/textures/lq_medieval/med_dr3b.png", "start": 188843679, "end": 188857987}, {"filename": "/GameData/textures/lq_medieval/med_dr3b_blu.png", "start": 188857987, "end": 188872193}, {"filename": "/GameData/textures/lq_medieval/med_dr3c.png", "start": 188872193, "end": 188885036}, {"filename": "/GameData/textures/lq_medieval/med_dr3c_blu.png", "start": 188885036, "end": 188898160}, {"filename": "/GameData/textures/lq_medieval/med_dwall1.png", "start": 188898160, "end": 188900636}, {"filename": "/GameData/textures/lq_medieval/med_ebrick1.png", "start": 188900636, "end": 188913218}, {"filename": "/GameData/textures/lq_medieval/med_ebrick10.png", "start": 188913218, "end": 188924103}, {"filename": "/GameData/textures/lq_medieval/med_ebrick10b.png", "start": 188924103, "end": 188934589}, {"filename": "/GameData/textures/lq_medieval/med_ebrick11.png", "start": 188934589, "end": 188949848}, {"filename": "/GameData/textures/lq_medieval/med_ebrick12.png", "start": 188949848, "end": 188967265}, {"filename": "/GameData/textures/lq_medieval/med_ebrick12b.png", "start": 188967265, "end": 188980504}, {"filename": "/GameData/textures/lq_medieval/med_ebrick13.png", "start": 188980504, "end": 188990402}, {"filename": "/GameData/textures/lq_medieval/med_ebrick14.png", "start": 188990402, "end": 188999417}, {"filename": "/GameData/textures/lq_medieval/med_ebrick15.png", "start": 188999417, "end": 189014218}, {"filename": "/GameData/textures/lq_medieval/med_ebrick16.png", "start": 189014218, "end": 189025582}, {"filename": "/GameData/textures/lq_medieval/med_ebrick16b.png", "start": 189025582, "end": 189033900}, {"filename": "/GameData/textures/lq_medieval/med_ebrick17.png", "start": 189033900, "end": 189047034}, {"filename": "/GameData/textures/lq_medieval/med_ebrick17b.png", "start": 189047034, "end": 189059300}, {"filename": "/GameData/textures/lq_medieval/med_ebrick17c.png", "start": 189059300, "end": 189072559}, {"filename": "/GameData/textures/lq_medieval/med_ebrick18.png", "start": 189072559, "end": 189088559}, {"filename": "/GameData/textures/lq_medieval/med_ebrick18b.png", "start": 189088559, "end": 189102947}, {"filename": "/GameData/textures/lq_medieval/med_ebrick2.png", "start": 189102947, "end": 189115439}, {"filename": "/GameData/textures/lq_medieval/med_ebrick20.png", "start": 189115439, "end": 189128839}, {"filename": "/GameData/textures/lq_medieval/med_ebrick21.png", "start": 189128839, "end": 189140981}, {"filename": "/GameData/textures/lq_medieval/med_ebrick22.png", "start": 189140981, "end": 189153142}, {"filename": "/GameData/textures/lq_medieval/med_ebrick3.png", "start": 189153142, "end": 189165602}, {"filename": "/GameData/textures/lq_medieval/med_ebrick4.png", "start": 189165602, "end": 189181152}, {"filename": "/GameData/textures/lq_medieval/med_ebrick5.png", "start": 189181152, "end": 189194968}, {"filename": "/GameData/textures/lq_medieval/med_ebrick6.png", "start": 189194968, "end": 189209679}, {"filename": "/GameData/textures/lq_medieval/med_ebrick7.png", "start": 189209679, "end": 189223324}, {"filename": "/GameData/textures/lq_medieval/med_ebrick8.png", "start": 189223324, "end": 189236063}, {"filename": "/GameData/textures/lq_medieval/med_ebrick9.png", "start": 189236063, "end": 189250618}, {"filename": "/GameData/textures/lq_medieval/med_etrim1.png", "start": 189250618, "end": 189253947}, {"filename": "/GameData/textures/lq_medieval/med_glass1.png", "start": 189253947, "end": 189266326}, {"filename": "/GameData/textures/lq_medieval/med_glass2.png", "start": 189266326, "end": 189277983}, {"filename": "/GameData/textures/lq_medieval/med_glass3.png", "start": 189277983, "end": 189288998}, {"filename": "/GameData/textures/lq_medieval/med_glass4.png", "start": 189288998, "end": 189320817}, {"filename": "/GameData/textures/lq_medieval/med_glass5.png", "start": 189320817, "end": 189330599}, {"filename": "/GameData/textures/lq_medieval/med_met_dec1.png", "start": 189330599, "end": 189334739}, {"filename": "/GameData/textures/lq_medieval/med_met_key1a.png", "start": 189334739, "end": 189336499}, {"filename": "/GameData/textures/lq_medieval/med_met_key1b.png", "start": 189336499, "end": 189337833}, {"filename": "/GameData/textures/lq_medieval/med_met_key2a.png", "start": 189337833, "end": 189339593}, {"filename": "/GameData/textures/lq_medieval/med_met_key2b.png", "start": 189339593, "end": 189341299}, {"filename": "/GameData/textures/lq_medieval/med_met_trim1.png", "start": 189341299, "end": 189342326}, {"filename": "/GameData/textures/lq_medieval/med_met_trim2.png", "start": 189342326, "end": 189343273}, {"filename": "/GameData/textures/lq_medieval/med_met_trim3.png", "start": 189343273, "end": 189344147}, {"filename": "/GameData/textures/lq_medieval/med_metw1a.png", "start": 189344147, "end": 189356455}, {"filename": "/GameData/textures/lq_medieval/med_metw1b.png", "start": 189356455, "end": 189365162}, {"filename": "/GameData/textures/lq_medieval/med_metw2a.png", "start": 189365162, "end": 189375069}, {"filename": "/GameData/textures/lq_medieval/med_metw2b.png", "start": 189375069, "end": 189387447}, {"filename": "/GameData/textures/lq_medieval/med_rmet.png", "start": 189387447, "end": 189400548}, {"filename": "/GameData/textures/lq_medieval/med_rmet_slat.png", "start": 189400548, "end": 189414859}, {"filename": "/GameData/textures/lq_medieval/med_rmet_tile.png", "start": 189414859, "end": 189428791}, {"filename": "/GameData/textures/lq_medieval/med_rmet_trim32.png", "start": 189428791, "end": 189442000}, {"filename": "/GameData/textures/lq_medieval/med_roof1.png", "start": 189442000, "end": 189453915}, {"filename": "/GameData/textures/lq_medieval/med_roof2.png", "start": 189453915, "end": 189468988}, {"filename": "/GameData/textures/lq_medieval/med_roof3.png", "start": 189468988, "end": 189483917}, {"filename": "/GameData/textures/lq_medieval/med_roof4.png", "start": 189483917, "end": 189501731}, {"filename": "/GameData/textures/lq_medieval/med_roof5.png", "start": 189501731, "end": 189512237}, {"filename": "/GameData/textures/lq_medieval/med_tanwall1.png", "start": 189512237, "end": 189553915}, {"filename": "/GameData/textures/lq_medieval/med_tanwall2.png", "start": 189553915, "end": 189613692}, {"filename": "/GameData/textures/lq_medieval/med_tanwall3.png", "start": 189613692, "end": 189664127}, {"filename": "/GameData/textures/lq_medieval/med_tanwall4.png", "start": 189664127, "end": 189713268}, {"filename": "/GameData/textures/lq_medieval/med_tanwall4_f.png", "start": 189713268, "end": 189757221}, {"filename": "/GameData/textures/lq_medieval/med_tanwall6.png", "start": 189757221, "end": 189801766}, {"filename": "/GameData/textures/lq_medieval/med_tanwall7.png", "start": 189801766, "end": 189847359}, {"filename": "/GameData/textures/lq_medieval/med_tanwall8.png", "start": 189847359, "end": 189884957}, {"filename": "/GameData/textures/lq_medieval/med_tanwall9.png", "start": 189884957, "end": 189934037}, {"filename": "/GameData/textures/lq_medieval/med_tanwall9_f.png", "start": 189934037, "end": 189978016}, {"filename": "/GameData/textures/lq_medieval/med_telepad.png", "start": 189978016, "end": 189982643}, {"filename": "/GameData/textures/lq_medieval/med_tile1.png", "start": 189982643, "end": 190055254}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_lit1_fbr.png", "start": 190055254, "end": 190056857}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_lit3_fbr.png", "start": 190056857, "end": 190058162}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_tele.png", "start": 190058162, "end": 190061344}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim1.png", "start": 190061344, "end": 190064510}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim1b.png", "start": 190064510, "end": 190066565}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim2.png", "start": 190066565, "end": 190070105}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim3.png", "start": 190070105, "end": 190073582}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim4.png", "start": 190073582, "end": 190076695}, {"filename": "/GameData/textures/lq_medieval/med_trim1_1.png", "start": 190076695, "end": 190085399}, {"filename": "/GameData/textures/lq_medieval/med_trim2_1.png", "start": 190085399, "end": 190086513}, {"filename": "/GameData/textures/lq_medieval/med_trim3_1.png", "start": 190086513, "end": 190090108}, {"filename": "/GameData/textures/lq_medieval/med_trim3_2.png", "start": 190090108, "end": 190093667}, {"filename": "/GameData/textures/lq_medieval/med_trim3_3.png", "start": 190093667, "end": 190097364}, {"filename": "/GameData/textures/lq_medieval/med_trim3_4.png", "start": 190097364, "end": 190100735}, {"filename": "/GameData/textures/lq_medieval/med_trim4_1.png", "start": 190100735, "end": 190104159}, {"filename": "/GameData/textures/lq_medieval/med_trim4_2.png", "start": 190104159, "end": 190107683}, {"filename": "/GameData/textures/lq_medieval/med_trim4_3.png", "start": 190107683, "end": 190111043}, {"filename": "/GameData/textures/lq_medieval/med_trim4_4.png", "start": 190111043, "end": 190114266}, {"filename": "/GameData/textures/lq_medieval/plus_0_csl_brk14.png", "start": 190114266, "end": 190130579}, {"filename": "/GameData/textures/lq_medieval/plus_1_csl_brk14.png", "start": 190130579, "end": 190146872}, {"filename": "/GameData/textures/lq_medieval/plus_2_csl_brk14.png", "start": 190146872, "end": 190163171}, {"filename": "/GameData/textures/lq_medieval/plus_3_csl_brk14.png", "start": 190163171, "end": 190179413}, {"filename": "/GameData/textures/lq_medieval/plus_4_csl_brk14.png", "start": 190179413, "end": 190195754}, {"filename": "/GameData/textures/lq_medieval/sidewalk.png", "start": 190195754, "end": 190204510}, {"filename": "/GameData/textures/lq_medieval/sq_trim1_2.png", "start": 190204510, "end": 190212792}, {"filename": "/GameData/textures/lq_medieval/sq_trim1_2_s.png", "start": 190212792, "end": 190215367}, {"filename": "/GameData/textures/lq_medieval/tile.png", "start": 190215367, "end": 190218492}, {"filename": "/GameData/textures/lq_medieval/tile1.png", "start": 190218492, "end": 190291105}, {"filename": "/GameData/textures/lq_medieval/wall14_5.png", "start": 190291105, "end": 190295185}, {"filename": "/GameData/textures/lq_medieval/wbrick1_5.png", "start": 190295185, "end": 190298951}, {"filename": "/GameData/textures/lq_medieval/wswamp2_1.png", "start": 190298951, "end": 190302077}, {"filename": "/GameData/textures/lq_medieval/wswamp2_2.png", "start": 190302077, "end": 190305854}, {"filename": "/GameData/textures/lq_metal/gig1_bone.png", "start": 190305854, "end": 190315535}, {"filename": "/GameData/textures/lq_metal/gig1_bone_l.png", "start": 190315535, "end": 190346196}, {"filename": "/GameData/textures/lq_metal/gig1_skull.png", "start": 190346196, "end": 190348860}, {"filename": "/GameData/textures/lq_metal/gig1_skull_l.png", "start": 190348860, "end": 190357645}, {"filename": "/GameData/textures/lq_metal/gig1_spine.png", "start": 190357645, "end": 190406346}, {"filename": "/GameData/textures/lq_metal/gig2_bone.png", "start": 190406346, "end": 190414769}, {"filename": "/GameData/textures/lq_metal/gig2_bone_l.png", "start": 190414769, "end": 190440517}, {"filename": "/GameData/textures/lq_metal/gig2_bone_s.png", "start": 190440517, "end": 190443130}, {"filename": "/GameData/textures/lq_metal/gig2_mouth_s.png", "start": 190443130, "end": 190445545}, {"filename": "/GameData/textures/lq_metal/med_flat8.png", "start": 190445545, "end": 190448403}, {"filename": "/GameData/textures/lq_metal/med_flat9.png", "start": 190448403, "end": 190451391}, {"filename": "/GameData/textures/lq_metal/met_blc_block.png", "start": 190451391, "end": 190454730}, {"filename": "/GameData/textures/lq_metal/met_blc_diam.png", "start": 190454730, "end": 190457620}, {"filename": "/GameData/textures/lq_metal/met_blc_trim28.png", "start": 190457620, "end": 190462663}, {"filename": "/GameData/textures/lq_metal/met_blc_trim32.png", "start": 190462663, "end": 190465809}, {"filename": "/GameData/textures/lq_metal/met_blc_trim32r.png", "start": 190465809, "end": 190469064}, {"filename": "/GameData/textures/lq_metal/met_blc_trim32s.png", "start": 190469064, "end": 190472164}, {"filename": "/GameData/textures/lq_metal/met_blc_trim64.png", "start": 190472164, "end": 190475307}, {"filename": "/GameData/textures/lq_metal/met_blu_block.png", "start": 190475307, "end": 190480327}, {"filename": "/GameData/textures/lq_metal/met_blu_det1.png", "start": 190480327, "end": 190481483}, {"filename": "/GameData/textures/lq_metal/met_blu_diam.png", "start": 190481483, "end": 190484380}, {"filename": "/GameData/textures/lq_metal/met_blu_diam2.png", "start": 190484380, "end": 190487175}, {"filename": "/GameData/textures/lq_metal/met_blu_diamc.png", "start": 190487175, "end": 190490706}, {"filename": "/GameData/textures/lq_metal/met_blu_door1.png", "start": 190490706, "end": 190495389}, {"filename": "/GameData/textures/lq_metal/met_blu_door2.png", "start": 190495389, "end": 190499888}, {"filename": "/GameData/textures/lq_metal/met_blu_door3.png", "start": 190499888, "end": 190504537}, {"filename": "/GameData/textures/lq_metal/met_blu_door4.png", "start": 190504537, "end": 190509101}, {"filename": "/GameData/textures/lq_metal/met_blu_door5.png", "start": 190509101, "end": 190512044}, {"filename": "/GameData/textures/lq_metal/met_blu_door6.png", "start": 190512044, "end": 190514640}, {"filename": "/GameData/textures/lq_metal/met_blu_fac1.png", "start": 190514640, "end": 190517042}, {"filename": "/GameData/textures/lq_metal/met_blu_flat.png", "start": 190517042, "end": 190520476}, {"filename": "/GameData/textures/lq_metal/met_blu_flatst.png", "start": 190520476, "end": 190522705}, {"filename": "/GameData/textures/lq_metal/met_blu_gig1.png", "start": 190522705, "end": 190526824}, {"filename": "/GameData/textures/lq_metal/met_blu_gig2.png", "start": 190526824, "end": 190530437}, {"filename": "/GameData/textures/lq_metal/met_blu_gig2b.png", "start": 190530437, "end": 190536631}, {"filename": "/GameData/textures/lq_metal/met_blu_grate.png", "start": 190536631, "end": 190539686}, {"filename": "/GameData/textures/lq_metal/met_blu_grate2.png", "start": 190539686, "end": 190542517}, {"filename": "/GameData/textures/lq_metal/met_blu_grate3.png", "start": 190542517, "end": 190544177}, {"filename": "/GameData/textures/lq_metal/met_blu_lit1_fbr.png", "start": 190544177, "end": 190546248}, {"filename": "/GameData/textures/lq_metal/met_blu_lit2_fbr.png", "start": 190546248, "end": 190547437}, {"filename": "/GameData/textures/lq_metal/met_blu_lit3.png", "start": 190547437, "end": 190548623}, {"filename": "/GameData/textures/lq_metal/met_blu_lit4.png", "start": 190548623, "end": 190549884}, {"filename": "/GameData/textures/lq_metal/met_blu_lit5.png", "start": 190549884, "end": 190551977}, {"filename": "/GameData/textures/lq_metal/met_blu_pan1.png", "start": 190551977, "end": 190555459}, {"filename": "/GameData/textures/lq_metal/met_blu_pan2.png", "start": 190555459, "end": 190557745}, {"filename": "/GameData/textures/lq_metal/met_blu_pan3.png", "start": 190557745, "end": 190561172}, {"filename": "/GameData/textures/lq_metal/met_blu_rect.png", "start": 190561172, "end": 190564951}, {"filename": "/GameData/textures/lq_metal/met_blu_rivg.png", "start": 190564951, "end": 190568872}, {"filename": "/GameData/textures/lq_metal/met_blu_rivs.png", "start": 190568872, "end": 190572230}, {"filename": "/GameData/textures/lq_metal/met_blu_slat.png", "start": 190572230, "end": 190576016}, {"filename": "/GameData/textures/lq_metal/met_blu_sqr.png", "start": 190576016, "end": 190579787}, {"filename": "/GameData/textures/lq_metal/met_blu_sqrd.png", "start": 190579787, "end": 190583625}, {"filename": "/GameData/textures/lq_metal/met_blu_sqrs.png", "start": 190583625, "end": 190587379}, {"filename": "/GameData/textures/lq_metal/met_blu_stile.png", "start": 190587379, "end": 190590606}, {"filename": "/GameData/textures/lq_metal/met_blu_tile.png", "start": 190590606, "end": 190594055}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16.png", "start": 190594055, "end": 190597759}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16g.png", "start": 190597759, "end": 190601509}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16h.png", "start": 190601509, "end": 190605126}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16s.png", "start": 190605126, "end": 190608294}, {"filename": "/GameData/textures/lq_metal/met_blu_trim28.png", "start": 190608294, "end": 190613591}, {"filename": "/GameData/textures/lq_metal/met_blu_trim32.png", "start": 190613591, "end": 190617041}, {"filename": "/GameData/textures/lq_metal/met_blu_trim32r.png", "start": 190617041, "end": 190620697}, {"filename": "/GameData/textures/lq_metal/met_blu_trim32s.png", "start": 190620697, "end": 190624021}, {"filename": "/GameData/textures/lq_metal/met_blu_trim64.png", "start": 190624021, "end": 190627498}, {"filename": "/GameData/textures/lq_metal/met_blu_vtrim.png", "start": 190627498, "end": 190631068}, {"filename": "/GameData/textures/lq_metal/met_brn2_pat.png", "start": 190631068, "end": 190634917}, {"filename": "/GameData/textures/lq_metal/met_brn_block.png", "start": 190634917, "end": 190638628}, {"filename": "/GameData/textures/lq_metal/met_brn_blockl.png", "start": 190638628, "end": 190642157}, {"filename": "/GameData/textures/lq_metal/met_brn_det1.png", "start": 190642157, "end": 190643320}, {"filename": "/GameData/textures/lq_metal/met_brn_flat.png", "start": 190643320, "end": 190646841}, {"filename": "/GameData/textures/lq_metal/met_brn_grate.png", "start": 190646841, "end": 190650229}, {"filename": "/GameData/textures/lq_metal/met_brn_grate2.png", "start": 190650229, "end": 190653735}, {"filename": "/GameData/textures/lq_metal/met_brn_grate3.png", "start": 190653735, "end": 190655753}, {"filename": "/GameData/textures/lq_metal/met_brn_lit1_fbr.png", "start": 190655753, "end": 190657866}, {"filename": "/GameData/textures/lq_metal/met_brn_lit2_fbr.png", "start": 190657866, "end": 190659054}, {"filename": "/GameData/textures/lq_metal/met_brn_lit3.png", "start": 190659054, "end": 190660225}, {"filename": "/GameData/textures/lq_metal/met_brn_lit4.png", "start": 190660225, "end": 190661435}, {"filename": "/GameData/textures/lq_metal/met_brn_lit5.png", "start": 190661435, "end": 190663488}, {"filename": "/GameData/textures/lq_metal/met_brn_pan1.png", "start": 190663488, "end": 190666951}, {"filename": "/GameData/textures/lq_metal/met_brn_pan2.png", "start": 190666951, "end": 190669227}, {"filename": "/GameData/textures/lq_metal/met_brn_pan3.png", "start": 190669227, "end": 190672704}, {"filename": "/GameData/textures/lq_metal/met_brn_pan4.png", "start": 190672704, "end": 190676567}, {"filename": "/GameData/textures/lq_metal/met_brn_rect.png", "start": 190676567, "end": 190680245}, {"filename": "/GameData/textures/lq_metal/met_brn_rivg.png", "start": 190680245, "end": 190684273}, {"filename": "/GameData/textures/lq_metal/met_brn_rivs.png", "start": 190684273, "end": 190687696}, {"filename": "/GameData/textures/lq_metal/met_brn_signs.png", "start": 190687696, "end": 190692530}, {"filename": "/GameData/textures/lq_metal/met_brn_slat.png", "start": 190692530, "end": 190696173}, {"filename": "/GameData/textures/lq_metal/met_brn_sqr.png", "start": 190696173, "end": 190699864}, {"filename": "/GameData/textures/lq_metal/met_brn_sqrd.png", "start": 190699864, "end": 190703672}, {"filename": "/GameData/textures/lq_metal/met_brn_sqrs.png", "start": 190703672, "end": 190707376}, {"filename": "/GameData/textures/lq_metal/met_brn_stile.png", "start": 190707376, "end": 190711461}, {"filename": "/GameData/textures/lq_metal/met_brn_tile.png", "start": 190711461, "end": 190714842}, {"filename": "/GameData/textures/lq_metal/met_brn_tile2.png", "start": 190714842, "end": 190718175}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16.png", "start": 190718175, "end": 190722281}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16g.png", "start": 190722281, "end": 190726485}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16h.png", "start": 190726485, "end": 190730682}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16s.png", "start": 190730682, "end": 190734305}, {"filename": "/GameData/textures/lq_metal/met_brn_trim32.png", "start": 190734305, "end": 190737792}, {"filename": "/GameData/textures/lq_metal/met_brn_trim32s.png", "start": 190737792, "end": 190741178}, {"filename": "/GameData/textures/lq_metal/met_brn_trim64.png", "start": 190741178, "end": 190744818}, {"filename": "/GameData/textures/lq_metal/met_brn_vtrim.png", "start": 190744818, "end": 190748271}, {"filename": "/GameData/textures/lq_metal/met_cop_flat.png", "start": 190748271, "end": 190751348}, {"filename": "/GameData/textures/lq_metal/met_cop_riv.png", "start": 190751348, "end": 190754949}, {"filename": "/GameData/textures/lq_metal/met_dbrn_flat.png", "start": 190754949, "end": 190758329}, {"filename": "/GameData/textures/lq_metal/met_dbrn_rect.png", "start": 190758329, "end": 190762214}, {"filename": "/GameData/textures/lq_metal/met_dbrn_slat.png", "start": 190762214, "end": 190766029}, {"filename": "/GameData/textures/lq_metal/met_grate.png", "start": 190766029, "end": 190769379}, {"filename": "/GameData/textures/lq_metal/met_grn_block.png", "start": 190769379, "end": 190773304}, {"filename": "/GameData/textures/lq_metal/met_grn_blockl.png", "start": 190773304, "end": 190777033}, {"filename": "/GameData/textures/lq_metal/met_grn_det1.png", "start": 190777033, "end": 190778284}, {"filename": "/GameData/textures/lq_metal/met_grn_fac1.png", "start": 190778284, "end": 190781194}, {"filename": "/GameData/textures/lq_metal/met_grn_flat.png", "start": 190781194, "end": 190785079}, {"filename": "/GameData/textures/lq_metal/met_grn_grate.png", "start": 190785079, "end": 190788141}, {"filename": "/GameData/textures/lq_metal/met_grn_grate2.png", "start": 190788141, "end": 190791124}, {"filename": "/GameData/textures/lq_metal/met_grn_grate3.png", "start": 190791124, "end": 190792868}, {"filename": "/GameData/textures/lq_metal/met_grn_lit1_fbr.png", "start": 190792868, "end": 190795002}, {"filename": "/GameData/textures/lq_metal/met_grn_lit2_fbr.png", "start": 190795002, "end": 190796203}, {"filename": "/GameData/textures/lq_metal/met_grn_lit3.png", "start": 190796203, "end": 190797415}, {"filename": "/GameData/textures/lq_metal/met_grn_lit4.png", "start": 190797415, "end": 190798701}, {"filename": "/GameData/textures/lq_metal/met_grn_lit5.png", "start": 190798701, "end": 190800893}, {"filename": "/GameData/textures/lq_metal/met_grn_pan1.png", "start": 190800893, "end": 190804609}, {"filename": "/GameData/textures/lq_metal/met_grn_pan2.png", "start": 190804609, "end": 190806974}, {"filename": "/GameData/textures/lq_metal/met_grn_pan3.png", "start": 190806974, "end": 190810574}, {"filename": "/GameData/textures/lq_metal/met_grn_rect.png", "start": 190810574, "end": 190814611}, {"filename": "/GameData/textures/lq_metal/met_grn_rivg.png", "start": 190814611, "end": 190819023}, {"filename": "/GameData/textures/lq_metal/met_grn_rivs.png", "start": 190819023, "end": 190822790}, {"filename": "/GameData/textures/lq_metal/met_grn_slat.png", "start": 190822790, "end": 190826831}, {"filename": "/GameData/textures/lq_metal/met_grn_sqr.png", "start": 190826831, "end": 190830829}, {"filename": "/GameData/textures/lq_metal/met_grn_sqrd.png", "start": 190830829, "end": 190834939}, {"filename": "/GameData/textures/lq_metal/met_grn_sqrs.png", "start": 190834939, "end": 190838969}, {"filename": "/GameData/textures/lq_metal/met_grn_stile.png", "start": 190838969, "end": 190842253}, {"filename": "/GameData/textures/lq_metal/met_grn_tile.png", "start": 190842253, "end": 190845514}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16.png", "start": 190845514, "end": 190849126}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16g.png", "start": 190849126, "end": 190853280}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16h.png", "start": 190853280, "end": 190856829}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16s.png", "start": 190856829, "end": 190860340}, {"filename": "/GameData/textures/lq_metal/met_grn_trim28.png", "start": 190860340, "end": 190865275}, {"filename": "/GameData/textures/lq_metal/met_grn_trim28r.png", "start": 190865275, "end": 190870301}, {"filename": "/GameData/textures/lq_metal/met_grn_trim32.png", "start": 190870301, "end": 190874126}, {"filename": "/GameData/textures/lq_metal/met_grn_trim32r.png", "start": 190874126, "end": 190877996}, {"filename": "/GameData/textures/lq_metal/met_grn_trim32s.png", "start": 190877996, "end": 190881642}, {"filename": "/GameData/textures/lq_metal/met_grn_trim64.png", "start": 190881642, "end": 190885299}, {"filename": "/GameData/textures/lq_metal/met_grn_vtrim.png", "start": 190885299, "end": 190888549}, {"filename": "/GameData/textures/lq_metal/met_gry_beam.png", "start": 190888549, "end": 190891563}, {"filename": "/GameData/textures/lq_metal/met_gry_block.png", "start": 190891563, "end": 190894586}, {"filename": "/GameData/textures/lq_metal/met_gry_flat.png", "start": 190894586, "end": 190897320}, {"filename": "/GameData/textures/lq_metal/met_gry_lit1_fbr.png", "start": 190897320, "end": 190899131}, {"filename": "/GameData/textures/lq_metal/met_gry_lit2_fbr.png", "start": 190899131, "end": 190900254}, {"filename": "/GameData/textures/lq_metal/met_gry_lit2b.png", "start": 190900254, "end": 190903704}, {"filename": "/GameData/textures/lq_metal/met_gry_pan1.png", "start": 190903704, "end": 190906800}, {"filename": "/GameData/textures/lq_metal/met_gry_pan2.png", "start": 190906800, "end": 190908953}, {"filename": "/GameData/textures/lq_metal/met_gry_pan3.png", "start": 190908953, "end": 190912056}, {"filename": "/GameData/textures/lq_metal/met_gry_rect.png", "start": 190912056, "end": 190915253}, {"filename": "/GameData/textures/lq_metal/met_gry_signs.png", "start": 190915253, "end": 190919083}, {"filename": "/GameData/textures/lq_metal/met_gry_slat.png", "start": 190919083, "end": 190922499}, {"filename": "/GameData/textures/lq_metal/met_gry_sqr.png", "start": 190922499, "end": 190925333}, {"filename": "/GameData/textures/lq_metal/met_gry_sqrd.png", "start": 190925333, "end": 190928581}, {"filename": "/GameData/textures/lq_metal/met_gry_sqrs.png", "start": 190928581, "end": 190931542}, {"filename": "/GameData/textures/lq_metal/met_gry_trim64.png", "start": 190931542, "end": 190935243}, {"filename": "/GameData/textures/lq_metal/met_lbrn_flat.png", "start": 190935243, "end": 190938669}, {"filename": "/GameData/textures/lq_metal/met_lbrn_rect.png", "start": 190938669, "end": 190942557}, {"filename": "/GameData/textures/lq_metal/met_lbrn_slat.png", "start": 190942557, "end": 190946511}, {"filename": "/GameData/textures/lq_metal/met_lift.png", "start": 190946511, "end": 190949556}, {"filename": "/GameData/textures/lq_metal/met_met7_1.png", "start": 190949556, "end": 190952029}, {"filename": "/GameData/textures/lq_metal/met_mix_beam.png", "start": 190952029, "end": 190955681}, {"filename": "/GameData/textures/lq_metal/met_mix_diam.png", "start": 190955681, "end": 190958870}, {"filename": "/GameData/textures/lq_metal/met_mix_diam2.png", "start": 190958870, "end": 190962069}, {"filename": "/GameData/textures/lq_metal/met_mix_diamc.png", "start": 190962069, "end": 190966169}, {"filename": "/GameData/textures/lq_metal/met_mt1_flat.png", "start": 190966169, "end": 190980459}, {"filename": "/GameData/textures/lq_metal/met_mt1_rect.png", "start": 190980459, "end": 190996169}, {"filename": "/GameData/textures/lq_metal/met_mt1_slat.png", "start": 190996169, "end": 191011925}, {"filename": "/GameData/textures/lq_metal/met_mt1_sqr.png", "start": 191011925, "end": 191018465}, {"filename": "/GameData/textures/lq_metal/met_mt2_flat.png", "start": 191018465, "end": 191034679}, {"filename": "/GameData/textures/lq_metal/met_mt2_rect.png", "start": 191034679, "end": 191051405}, {"filename": "/GameData/textures/lq_metal/met_mt2_slat.png", "start": 191051405, "end": 191068043}, {"filename": "/GameData/textures/lq_metal/met_mt2_sqr.png", "start": 191068043, "end": 191076441}, {"filename": "/GameData/textures/lq_metal/met_mt3_flat.png", "start": 191076441, "end": 191093041}, {"filename": "/GameData/textures/lq_metal/met_mt3_rect.png", "start": 191093041, "end": 191109949}, {"filename": "/GameData/textures/lq_metal/met_mt3_slat.png", "start": 191109949, "end": 191126860}, {"filename": "/GameData/textures/lq_metal/met_mt3_sqr.png", "start": 191126860, "end": 191135395}, {"filename": "/GameData/textures/lq_metal/met_ora_trim64.png", "start": 191135395, "end": 191138174}, {"filename": "/GameData/textures/lq_metal/met_rail_flat.png", "start": 191138174, "end": 191140321}, {"filename": "/GameData/textures/lq_metal/met_rune1_fbr.png", "start": 191140321, "end": 191143125}, {"filename": "/GameData/textures/lq_metal/met_rune_trim32.png", "start": 191143125, "end": 191147466}, {"filename": "/GameData/textures/lq_metal/met_set1.png", "start": 191147466, "end": 191217311}, {"filename": "/GameData/textures/lq_metal/met_shm_flat.png", "start": 191217311, "end": 191220972}, {"filename": "/GameData/textures/lq_metal/met_shm_rect.png", "start": 191220972, "end": 191224844}, {"filename": "/GameData/textures/lq_metal/met_shm_slat.png", "start": 191224844, "end": 191228895}, {"filename": "/GameData/textures/lq_metal/met_shm_sqr.png", "start": 191228895, "end": 191232357}, {"filename": "/GameData/textures/lq_metal/met_teal_block.png", "start": 191232357, "end": 191235735}, {"filename": "/GameData/textures/lq_metal/met_teal_flat.png", "start": 191235735, "end": 191239081}, {"filename": "/GameData/textures/lq_metal/met_teal_trim32.png", "start": 191239081, "end": 191242357}, {"filename": "/GameData/textures/lq_metal/met_teal_trim32r.png", "start": 191242357, "end": 191245332}, {"filename": "/GameData/textures/lq_metal/met_teal_trim64.png", "start": 191245332, "end": 191248816}, {"filename": "/GameData/textures/lq_metal/met_wall3_1.png", "start": 191248816, "end": 191260082}, {"filename": "/GameData/textures/lq_metal/met_wall3_1_s.png", "start": 191260082, "end": 191263366}, {"filename": "/GameData/textures/lq_metal/metal4_4.png", "start": 191263366, "end": 191267707}, {"filename": "/GameData/textures/lq_metal/plus_0_sqbut1.png", "start": 191267707, "end": 191268789}, {"filename": "/GameData/textures/lq_metal/plus_0_sqbut2_fbr.png", "start": 191268789, "end": 191272025}, {"filename": "/GameData/textures/lq_metal/plus_0_sqshoot1_fbr.png", "start": 191272025, "end": 191273132}, {"filename": "/GameData/textures/lq_metal/plus_0gig2a_fbr.png", "start": 191273132, "end": 191273605}, {"filename": "/GameData/textures/lq_metal/plus_0gig_shot_fbr.png", "start": 191273605, "end": 191274081}, {"filename": "/GameData/textures/lq_metal/plus_0gig_sshot_fbr.png", "start": 191274081, "end": 191274525}, {"filename": "/GameData/textures/lq_metal/plus_0gig_ye_fbr.png", "start": 191274525, "end": 191274965}, {"filename": "/GameData/textures/lq_metal/plus_0met_blu_keyg_fbr.png", "start": 191274965, "end": 191275865}, {"filename": "/GameData/textures/lq_metal/plus_0met_blu_keys_fbr.png", "start": 191275865, "end": 191276748}, {"filename": "/GameData/textures/lq_metal/plus_1_sqbut1.png", "start": 191276748, "end": 191277914}, {"filename": "/GameData/textures/lq_metal/plus_1_sqbut2_fbr.png", "start": 191277914, "end": 191281102}, {"filename": "/GameData/textures/lq_metal/plus_1_sqshoot1.png", "start": 191281102, "end": 191282216}, {"filename": "/GameData/textures/lq_metal/plus_1met_blu_keyg_fbr.png", "start": 191282216, "end": 191283117}, {"filename": "/GameData/textures/lq_metal/plus_1met_blu_keys_fbr.png", "start": 191283117, "end": 191283994}, {"filename": "/GameData/textures/lq_metal/plus_2met_blu_keyg_fbr.png", "start": 191283994, "end": 191284903}, {"filename": "/GameData/textures/lq_metal/plus_2met_blu_keys_fbr.png", "start": 191284903, "end": 191285772}, {"filename": "/GameData/textures/lq_metal/plus_3met_blu_keyg_fbr.png", "start": 191285772, "end": 191286680}, {"filename": "/GameData/textures/lq_metal/plus_3met_blu_keys_fbr.png", "start": 191286680, "end": 191287536}, {"filename": "/GameData/textures/lq_metal/plus_4met_blu_keyg_fbr.png", "start": 191287536, "end": 191288444}, {"filename": "/GameData/textures/lq_metal/plus_4met_blu_keys_fbr.png", "start": 191288444, "end": 191289300}, {"filename": "/GameData/textures/lq_metal/plus_5met_blu_keyg_fbr.png", "start": 191289300, "end": 191290209}, {"filename": "/GameData/textures/lq_metal/plus_5met_blu_keys_fbr.png", "start": 191290209, "end": 191291078}, {"filename": "/GameData/textures/lq_metal/plus_6met_blu_keyg_fbr.png", "start": 191291078, "end": 191291979}, {"filename": "/GameData/textures/lq_metal/plus_6met_blu_keys_fbr.png", "start": 191291979, "end": 191292856}, {"filename": "/GameData/textures/lq_metal/plus_a_sqbut1.png", "start": 191292856, "end": 191293938}, {"filename": "/GameData/textures/lq_metal/plus_a_sqbut2_fbr.png", "start": 191293938, "end": 191297174}, {"filename": "/GameData/textures/lq_metal/plus_a_sqshoot1_fbr.png", "start": 191297174, "end": 191298281}, {"filename": "/GameData/textures/lq_metal/plus_agig2a.png", "start": 191298281, "end": 191298651}, {"filename": "/GameData/textures/lq_metal/plus_agig_shot_fbr.png", "start": 191298651, "end": 191299026}, {"filename": "/GameData/textures/lq_metal/plus_agig_sshot_fbr.png", "start": 191299026, "end": 191299389}, {"filename": "/GameData/textures/lq_metal/plus_agig_ye.png", "start": 191299389, "end": 191299801}, {"filename": "/GameData/textures/lq_metal/plus_amet_blu_keyg.png", "start": 191299801, "end": 191300671}, {"filename": "/GameData/textures/lq_metal/plus_amet_blu_keys.png", "start": 191300671, "end": 191301541}, {"filename": "/GameData/textures/lq_metal/ret_metal1_tile.png", "start": 191301541, "end": 191315473}, {"filename": "/GameData/textures/lq_metal/sq_lit1_fbr.png", "start": 191315473, "end": 191315846}, {"filename": "/GameData/textures/lq_metal/sq_lit2_fbr.png", "start": 191315846, "end": 191316107}, {"filename": "/GameData/textures/lq_palette/flat_01_a.png", "start": 191316107, "end": 191316655}, {"filename": "/GameData/textures/lq_palette/flat_01_b.png", "start": 191316655, "end": 191317181}, {"filename": "/GameData/textures/lq_palette/flat_01_c.png", "start": 191317181, "end": 191317707}, {"filename": "/GameData/textures/lq_palette/flat_01_d.png", "start": 191317707, "end": 191318233}, {"filename": "/GameData/textures/lq_palette/flat_01_e.png", "start": 191318233, "end": 191318759}, {"filename": "/GameData/textures/lq_palette/flat_01_f.png", "start": 191318759, "end": 191319285}, {"filename": "/GameData/textures/lq_palette/flat_01_g.png", "start": 191319285, "end": 191319811}, {"filename": "/GameData/textures/lq_palette/flat_01_h.png", "start": 191319811, "end": 191320337}, {"filename": "/GameData/textures/lq_palette/flat_01_i.png", "start": 191320337, "end": 191320864}, {"filename": "/GameData/textures/lq_palette/flat_01_j.png", "start": 191320864, "end": 191321391}, {"filename": "/GameData/textures/lq_palette/flat_01_k.png", "start": 191321391, "end": 191321918}, {"filename": "/GameData/textures/lq_palette/flat_01_l.png", "start": 191321918, "end": 191322445}, {"filename": "/GameData/textures/lq_palette/flat_01_m.png", "start": 191322445, "end": 191322972}, {"filename": "/GameData/textures/lq_palette/flat_01_n.png", "start": 191322972, "end": 191323497}, {"filename": "/GameData/textures/lq_palette/flat_01_o.png", "start": 191323497, "end": 191324022}, {"filename": "/GameData/textures/lq_palette/flat_01_p.png", "start": 191324022, "end": 191324547}, {"filename": "/GameData/textures/lq_palette/flat_02_a.png", "start": 191324547, "end": 191325073}, {"filename": "/GameData/textures/lq_palette/flat_02_b.png", "start": 191325073, "end": 191325599}, {"filename": "/GameData/textures/lq_palette/flat_02_c.png", "start": 191325599, "end": 191326125}, {"filename": "/GameData/textures/lq_palette/flat_02_d.png", "start": 191326125, "end": 191326651}, {"filename": "/GameData/textures/lq_palette/flat_02_e.png", "start": 191326651, "end": 191327177}, {"filename": "/GameData/textures/lq_palette/flat_02_f.png", "start": 191327177, "end": 191327703}, {"filename": "/GameData/textures/lq_palette/flat_02_g.png", "start": 191327703, "end": 191328229}, {"filename": "/GameData/textures/lq_palette/flat_02_h.png", "start": 191328229, "end": 191328755}, {"filename": "/GameData/textures/lq_palette/flat_02_i.png", "start": 191328755, "end": 191329281}, {"filename": "/GameData/textures/lq_palette/flat_02_j.png", "start": 191329281, "end": 191329807}, {"filename": "/GameData/textures/lq_palette/flat_02_k.png", "start": 191329807, "end": 191330333}, {"filename": "/GameData/textures/lq_palette/flat_02_l.png", "start": 191330333, "end": 191330859}, {"filename": "/GameData/textures/lq_palette/flat_02_m.png", "start": 191330859, "end": 191331385}, {"filename": "/GameData/textures/lq_palette/flat_02_n.png", "start": 191331385, "end": 191331911}, {"filename": "/GameData/textures/lq_palette/flat_02_o.png", "start": 191331911, "end": 191332437}, {"filename": "/GameData/textures/lq_palette/flat_02_p.png", "start": 191332437, "end": 191332963}, {"filename": "/GameData/textures/lq_palette/flat_03_a.png", "start": 191332963, "end": 191333489}, {"filename": "/GameData/textures/lq_palette/flat_03_b.png", "start": 191333489, "end": 191334015}, {"filename": "/GameData/textures/lq_palette/flat_03_c.png", "start": 191334015, "end": 191334541}, {"filename": "/GameData/textures/lq_palette/flat_03_d.png", "start": 191334541, "end": 191335067}, {"filename": "/GameData/textures/lq_palette/flat_03_e.png", "start": 191335067, "end": 191335593}, {"filename": "/GameData/textures/lq_palette/flat_03_f.png", "start": 191335593, "end": 191336119}, {"filename": "/GameData/textures/lq_palette/flat_03_g.png", "start": 191336119, "end": 191336645}, {"filename": "/GameData/textures/lq_palette/flat_03_h.png", "start": 191336645, "end": 191337171}, {"filename": "/GameData/textures/lq_palette/flat_03_i.png", "start": 191337171, "end": 191337697}, {"filename": "/GameData/textures/lq_palette/flat_03_j.png", "start": 191337697, "end": 191338223}, {"filename": "/GameData/textures/lq_palette/flat_03_k.png", "start": 191338223, "end": 191338749}, {"filename": "/GameData/textures/lq_palette/flat_03_l.png", "start": 191338749, "end": 191339275}, {"filename": "/GameData/textures/lq_palette/flat_03_m.png", "start": 191339275, "end": 191339802}, {"filename": "/GameData/textures/lq_palette/flat_03_n.png", "start": 191339802, "end": 191340329}, {"filename": "/GameData/textures/lq_palette/flat_03_o.png", "start": 191340329, "end": 191340856}, {"filename": "/GameData/textures/lq_palette/flat_03_p.png", "start": 191340856, "end": 191341383}, {"filename": "/GameData/textures/lq_palette/flat_04_a.png", "start": 191341383, "end": 191341931}, {"filename": "/GameData/textures/lq_palette/flat_04_b.png", "start": 191341931, "end": 191342457}, {"filename": "/GameData/textures/lq_palette/flat_04_c.png", "start": 191342457, "end": 191342983}, {"filename": "/GameData/textures/lq_palette/flat_04_d.png", "start": 191342983, "end": 191343509}, {"filename": "/GameData/textures/lq_palette/flat_04_e.png", "start": 191343509, "end": 191344035}, {"filename": "/GameData/textures/lq_palette/flat_04_f.png", "start": 191344035, "end": 191344561}, {"filename": "/GameData/textures/lq_palette/flat_04_g.png", "start": 191344561, "end": 191345087}, {"filename": "/GameData/textures/lq_palette/flat_04_h.png", "start": 191345087, "end": 191345613}, {"filename": "/GameData/textures/lq_palette/flat_04_i.png", "start": 191345613, "end": 191346139}, {"filename": "/GameData/textures/lq_palette/flat_04_j.png", "start": 191346139, "end": 191346665}, {"filename": "/GameData/textures/lq_palette/flat_04_k.png", "start": 191346665, "end": 191347191}, {"filename": "/GameData/textures/lq_palette/flat_04_l.png", "start": 191347191, "end": 191347717}, {"filename": "/GameData/textures/lq_palette/flat_04_m.png", "start": 191347717, "end": 191348243}, {"filename": "/GameData/textures/lq_palette/flat_04_n.png", "start": 191348243, "end": 191348769}, {"filename": "/GameData/textures/lq_palette/flat_04_o.png", "start": 191348769, "end": 191349295}, {"filename": "/GameData/textures/lq_palette/flat_04_p.png", "start": 191349295, "end": 191349821}, {"filename": "/GameData/textures/lq_palette/flat_05_a.png", "start": 191349821, "end": 191350347}, {"filename": "/GameData/textures/lq_palette/flat_05_b.png", "start": 191350347, "end": 191350873}, {"filename": "/GameData/textures/lq_palette/flat_05_c.png", "start": 191350873, "end": 191351399}, {"filename": "/GameData/textures/lq_palette/flat_05_d.png", "start": 191351399, "end": 191351925}, {"filename": "/GameData/textures/lq_palette/flat_05_e.png", "start": 191351925, "end": 191352451}, {"filename": "/GameData/textures/lq_palette/flat_05_f.png", "start": 191352451, "end": 191352977}, {"filename": "/GameData/textures/lq_palette/flat_05_g.png", "start": 191352977, "end": 191353503}, {"filename": "/GameData/textures/lq_palette/flat_05_h.png", "start": 191353503, "end": 191354029}, {"filename": "/GameData/textures/lq_palette/flat_05_i.png", "start": 191354029, "end": 191354555}, {"filename": "/GameData/textures/lq_palette/flat_05_j.png", "start": 191354555, "end": 191355081}, {"filename": "/GameData/textures/lq_palette/flat_05_k.png", "start": 191355081, "end": 191355607}, {"filename": "/GameData/textures/lq_palette/flat_05_l.png", "start": 191355607, "end": 191356133}, {"filename": "/GameData/textures/lq_palette/flat_05_m.png", "start": 191356133, "end": 191356659}, {"filename": "/GameData/textures/lq_palette/flat_05_n.png", "start": 191356659, "end": 191357185}, {"filename": "/GameData/textures/lq_palette/flat_05_o.png", "start": 191357185, "end": 191357711}, {"filename": "/GameData/textures/lq_palette/flat_05_p.png", "start": 191357711, "end": 191358237}, {"filename": "/GameData/textures/lq_palette/flat_06_a.png", "start": 191358237, "end": 191358763}, {"filename": "/GameData/textures/lq_palette/flat_06_b.png", "start": 191358763, "end": 191359289}, {"filename": "/GameData/textures/lq_palette/flat_06_c.png", "start": 191359289, "end": 191359815}, {"filename": "/GameData/textures/lq_palette/flat_06_d.png", "start": 191359815, "end": 191360341}, {"filename": "/GameData/textures/lq_palette/flat_06_e.png", "start": 191360341, "end": 191360867}, {"filename": "/GameData/textures/lq_palette/flat_06_f.png", "start": 191360867, "end": 191361393}, {"filename": "/GameData/textures/lq_palette/flat_06_g.png", "start": 191361393, "end": 191361919}, {"filename": "/GameData/textures/lq_palette/flat_06_h.png", "start": 191361919, "end": 191362445}, {"filename": "/GameData/textures/lq_palette/flat_06_i.png", "start": 191362445, "end": 191362971}, {"filename": "/GameData/textures/lq_palette/flat_06_j.png", "start": 191362971, "end": 191363497}, {"filename": "/GameData/textures/lq_palette/flat_06_k.png", "start": 191363497, "end": 191364023}, {"filename": "/GameData/textures/lq_palette/flat_06_l.png", "start": 191364023, "end": 191364549}, {"filename": "/GameData/textures/lq_palette/flat_06_m.png", "start": 191364549, "end": 191365075}, {"filename": "/GameData/textures/lq_palette/flat_06_n.png", "start": 191365075, "end": 191365601}, {"filename": "/GameData/textures/lq_palette/flat_06_o.png", "start": 191365601, "end": 191366127}, {"filename": "/GameData/textures/lq_palette/flat_06_p.png", "start": 191366127, "end": 191366653}, {"filename": "/GameData/textures/lq_palette/flat_07_a.png", "start": 191366653, "end": 191367179}, {"filename": "/GameData/textures/lq_palette/flat_07_b.png", "start": 191367179, "end": 191367705}, {"filename": "/GameData/textures/lq_palette/flat_07_c.png", "start": 191367705, "end": 191368231}, {"filename": "/GameData/textures/lq_palette/flat_07_d.png", "start": 191368231, "end": 191368757}, {"filename": "/GameData/textures/lq_palette/flat_07_e.png", "start": 191368757, "end": 191369283}, {"filename": "/GameData/textures/lq_palette/flat_07_f.png", "start": 191369283, "end": 191369809}, {"filename": "/GameData/textures/lq_palette/flat_07_g.png", "start": 191369809, "end": 191370335}, {"filename": "/GameData/textures/lq_palette/flat_07_h.png", "start": 191370335, "end": 191370861}, {"filename": "/GameData/textures/lq_palette/flat_07_i.png", "start": 191370861, "end": 191371387}, {"filename": "/GameData/textures/lq_palette/flat_07_j.png", "start": 191371387, "end": 191371913}, {"filename": "/GameData/textures/lq_palette/flat_07_k.png", "start": 191371913, "end": 191372439}, {"filename": "/GameData/textures/lq_palette/flat_07_l.png", "start": 191372439, "end": 191372965}, {"filename": "/GameData/textures/lq_palette/flat_07_m.png", "start": 191372965, "end": 191373491}, {"filename": "/GameData/textures/lq_palette/flat_07_n.png", "start": 191373491, "end": 191374017}, {"filename": "/GameData/textures/lq_palette/flat_07_o.png", "start": 191374017, "end": 191374543}, {"filename": "/GameData/textures/lq_palette/flat_07_p.png", "start": 191374543, "end": 191375069}, {"filename": "/GameData/textures/lq_palette/flat_08_a.png", "start": 191375069, "end": 191375595}, {"filename": "/GameData/textures/lq_palette/flat_08_b.png", "start": 191375595, "end": 191376121}, {"filename": "/GameData/textures/lq_palette/flat_08_c.png", "start": 191376121, "end": 191376647}, {"filename": "/GameData/textures/lq_palette/flat_08_d.png", "start": 191376647, "end": 191377173}, {"filename": "/GameData/textures/lq_palette/flat_08_e.png", "start": 191377173, "end": 191377699}, {"filename": "/GameData/textures/lq_palette/flat_08_f.png", "start": 191377699, "end": 191378225}, {"filename": "/GameData/textures/lq_palette/flat_08_g.png", "start": 191378225, "end": 191378751}, {"filename": "/GameData/textures/lq_palette/flat_08_h.png", "start": 191378751, "end": 191379277}, {"filename": "/GameData/textures/lq_palette/flat_08_i.png", "start": 191379277, "end": 191379803}, {"filename": "/GameData/textures/lq_palette/flat_08_j.png", "start": 191379803, "end": 191380329}, {"filename": "/GameData/textures/lq_palette/flat_08_k.png", "start": 191380329, "end": 191380855}, {"filename": "/GameData/textures/lq_palette/flat_08_l.png", "start": 191380855, "end": 191381381}, {"filename": "/GameData/textures/lq_palette/flat_08_m.png", "start": 191381381, "end": 191381907}, {"filename": "/GameData/textures/lq_palette/flat_08_n.png", "start": 191381907, "end": 191382434}, {"filename": "/GameData/textures/lq_palette/flat_08_o.png", "start": 191382434, "end": 191382961}, {"filename": "/GameData/textures/lq_palette/flat_08_p.png", "start": 191382961, "end": 191383488}, {"filename": "/GameData/textures/lq_palette/flat_09_a.png", "start": 191383488, "end": 191384015}, {"filename": "/GameData/textures/lq_palette/flat_09_b.png", "start": 191384015, "end": 191384542}, {"filename": "/GameData/textures/lq_palette/flat_09_c.png", "start": 191384542, "end": 191385069}, {"filename": "/GameData/textures/lq_palette/flat_09_d.png", "start": 191385069, "end": 191385595}, {"filename": "/GameData/textures/lq_palette/flat_09_e.png", "start": 191385595, "end": 191386121}, {"filename": "/GameData/textures/lq_palette/flat_09_f.png", "start": 191386121, "end": 191386647}, {"filename": "/GameData/textures/lq_palette/flat_09_g.png", "start": 191386647, "end": 191387173}, {"filename": "/GameData/textures/lq_palette/flat_09_h.png", "start": 191387173, "end": 191387699}, {"filename": "/GameData/textures/lq_palette/flat_09_i.png", "start": 191387699, "end": 191388225}, {"filename": "/GameData/textures/lq_palette/flat_09_j.png", "start": 191388225, "end": 191388751}, {"filename": "/GameData/textures/lq_palette/flat_09_k.png", "start": 191388751, "end": 191389277}, {"filename": "/GameData/textures/lq_palette/flat_09_l.png", "start": 191389277, "end": 191389803}, {"filename": "/GameData/textures/lq_palette/flat_09_m.png", "start": 191389803, "end": 191390329}, {"filename": "/GameData/textures/lq_palette/flat_09_n.png", "start": 191390329, "end": 191390855}, {"filename": "/GameData/textures/lq_palette/flat_09_o.png", "start": 191390855, "end": 191391381}, {"filename": "/GameData/textures/lq_palette/flat_09_p.png", "start": 191391381, "end": 191391907}, {"filename": "/GameData/textures/lq_palette/flat_10_a.png", "start": 191391907, "end": 191392434}, {"filename": "/GameData/textures/lq_palette/flat_10_b.png", "start": 191392434, "end": 191392960}, {"filename": "/GameData/textures/lq_palette/flat_10_c.png", "start": 191392960, "end": 191393486}, {"filename": "/GameData/textures/lq_palette/flat_10_d.png", "start": 191393486, "end": 191394012}, {"filename": "/GameData/textures/lq_palette/flat_10_e.png", "start": 191394012, "end": 191394538}, {"filename": "/GameData/textures/lq_palette/flat_10_f.png", "start": 191394538, "end": 191395064}, {"filename": "/GameData/textures/lq_palette/flat_10_g.png", "start": 191395064, "end": 191395590}, {"filename": "/GameData/textures/lq_palette/flat_10_h.png", "start": 191395590, "end": 191396116}, {"filename": "/GameData/textures/lq_palette/flat_10_i.png", "start": 191396116, "end": 191396642}, {"filename": "/GameData/textures/lq_palette/flat_10_j.png", "start": 191396642, "end": 191397168}, {"filename": "/GameData/textures/lq_palette/flat_10_k.png", "start": 191397168, "end": 191397694}, {"filename": "/GameData/textures/lq_palette/flat_10_l.png", "start": 191397694, "end": 191398220}, {"filename": "/GameData/textures/lq_palette/flat_10_m.png", "start": 191398220, "end": 191398746}, {"filename": "/GameData/textures/lq_palette/flat_10_n.png", "start": 191398746, "end": 191399272}, {"filename": "/GameData/textures/lq_palette/flat_10_o.png", "start": 191399272, "end": 191399798}, {"filename": "/GameData/textures/lq_palette/flat_10_p.png", "start": 191399798, "end": 191400324}, {"filename": "/GameData/textures/lq_palette/flat_11_a.png", "start": 191400324, "end": 191400851}, {"filename": "/GameData/textures/lq_palette/flat_11_b.png", "start": 191400851, "end": 191401378}, {"filename": "/GameData/textures/lq_palette/flat_11_c.png", "start": 191401378, "end": 191401905}, {"filename": "/GameData/textures/lq_palette/flat_11_d.png", "start": 191401905, "end": 191402432}, {"filename": "/GameData/textures/lq_palette/flat_11_e.png", "start": 191402432, "end": 191402959}, {"filename": "/GameData/textures/lq_palette/flat_11_f.png", "start": 191402959, "end": 191403485}, {"filename": "/GameData/textures/lq_palette/flat_11_g.png", "start": 191403485, "end": 191404011}, {"filename": "/GameData/textures/lq_palette/flat_11_h.png", "start": 191404011, "end": 191404537}, {"filename": "/GameData/textures/lq_palette/flat_11_i.png", "start": 191404537, "end": 191405063}, {"filename": "/GameData/textures/lq_palette/flat_11_j.png", "start": 191405063, "end": 191405589}, {"filename": "/GameData/textures/lq_palette/flat_11_k.png", "start": 191405589, "end": 191406115}, {"filename": "/GameData/textures/lq_palette/flat_11_l.png", "start": 191406115, "end": 191406641}, {"filename": "/GameData/textures/lq_palette/flat_11_m.png", "start": 191406641, "end": 191407167}, {"filename": "/GameData/textures/lq_palette/flat_11_n.png", "start": 191407167, "end": 191407693}, {"filename": "/GameData/textures/lq_palette/flat_11_o.png", "start": 191407693, "end": 191408219}, {"filename": "/GameData/textures/lq_palette/flat_11_p.png", "start": 191408219, "end": 191408745}, {"filename": "/GameData/textures/lq_palette/flat_12_a.png", "start": 191408745, "end": 191409271}, {"filename": "/GameData/textures/lq_palette/flat_12_b.png", "start": 191409271, "end": 191409797}, {"filename": "/GameData/textures/lq_palette/flat_12_c.png", "start": 191409797, "end": 191410323}, {"filename": "/GameData/textures/lq_palette/flat_12_d.png", "start": 191410323, "end": 191410849}, {"filename": "/GameData/textures/lq_palette/flat_12_e.png", "start": 191410849, "end": 191411375}, {"filename": "/GameData/textures/lq_palette/flat_12_f.png", "start": 191411375, "end": 191411901}, {"filename": "/GameData/textures/lq_palette/flat_12_g.png", "start": 191411901, "end": 191412427}, {"filename": "/GameData/textures/lq_palette/flat_12_h.png", "start": 191412427, "end": 191412953}, {"filename": "/GameData/textures/lq_palette/flat_12_i.png", "start": 191412953, "end": 191413479}, {"filename": "/GameData/textures/lq_palette/flat_12_j.png", "start": 191413479, "end": 191414005}, {"filename": "/GameData/textures/lq_palette/flat_12_k.png", "start": 191414005, "end": 191414531}, {"filename": "/GameData/textures/lq_palette/flat_12_l.png", "start": 191414531, "end": 191415057}, {"filename": "/GameData/textures/lq_palette/flat_12_m.png", "start": 191415057, "end": 191415583}, {"filename": "/GameData/textures/lq_palette/flat_12_n.png", "start": 191415583, "end": 191416109}, {"filename": "/GameData/textures/lq_palette/flat_12_o.png", "start": 191416109, "end": 191416635}, {"filename": "/GameData/textures/lq_palette/flat_12_p.png", "start": 191416635, "end": 191417161}, {"filename": "/GameData/textures/lq_palette/flat_13_a.png", "start": 191417161, "end": 191417687}, {"filename": "/GameData/textures/lq_palette/flat_13_b.png", "start": 191417687, "end": 191418213}, {"filename": "/GameData/textures/lq_palette/flat_13_c.png", "start": 191418213, "end": 191418739}, {"filename": "/GameData/textures/lq_palette/flat_13_d.png", "start": 191418739, "end": 191419265}, {"filename": "/GameData/textures/lq_palette/flat_13_e.png", "start": 191419265, "end": 191419791}, {"filename": "/GameData/textures/lq_palette/flat_13_f.png", "start": 191419791, "end": 191420317}, {"filename": "/GameData/textures/lq_palette/flat_13_g.png", "start": 191420317, "end": 191420843}, {"filename": "/GameData/textures/lq_palette/flat_13_h.png", "start": 191420843, "end": 191421369}, {"filename": "/GameData/textures/lq_palette/flat_13_i.png", "start": 191421369, "end": 191421895}, {"filename": "/GameData/textures/lq_palette/flat_13_j.png", "start": 191421895, "end": 191422421}, {"filename": "/GameData/textures/lq_palette/flat_13_k.png", "start": 191422421, "end": 191422947}, {"filename": "/GameData/textures/lq_palette/flat_13_l.png", "start": 191422947, "end": 191423473}, {"filename": "/GameData/textures/lq_palette/flat_13_m.png", "start": 191423473, "end": 191423999}, {"filename": "/GameData/textures/lq_palette/flat_13_n.png", "start": 191423999, "end": 191424525}, {"filename": "/GameData/textures/lq_palette/flat_13_o.png", "start": 191424525, "end": 191425051}, {"filename": "/GameData/textures/lq_palette/flat_13_p.png", "start": 191425051, "end": 191425577}, {"filename": "/GameData/textures/lq_palette/flat_14_a.png", "start": 191425577, "end": 191426125}, {"filename": "/GameData/textures/lq_palette/flat_14_b.png", "start": 191426125, "end": 191426651}, {"filename": "/GameData/textures/lq_palette/flat_14_c.png", "start": 191426651, "end": 191427177}, {"filename": "/GameData/textures/lq_palette/flat_14_d.png", "start": 191427177, "end": 191427703}, {"filename": "/GameData/textures/lq_palette/flat_14_e.png", "start": 191427703, "end": 191428229}, {"filename": "/GameData/textures/lq_palette/flat_14_f.png", "start": 191428229, "end": 191428755}, {"filename": "/GameData/textures/lq_palette/flat_14_g.png", "start": 191428755, "end": 191429281}, {"filename": "/GameData/textures/lq_palette/flat_14_h.png", "start": 191429281, "end": 191429807}, {"filename": "/GameData/textures/lq_palette/flat_14_i.png", "start": 191429807, "end": 191430333}, {"filename": "/GameData/textures/lq_palette/flat_14_j.png", "start": 191430333, "end": 191430859}, {"filename": "/GameData/textures/lq_palette/flat_14_k.png", "start": 191430859, "end": 191431385}, {"filename": "/GameData/textures/lq_palette/flat_14_l.png", "start": 191431385, "end": 191431911}, {"filename": "/GameData/textures/lq_palette/flat_14_m.png", "start": 191431911, "end": 191432437}, {"filename": "/GameData/textures/lq_palette/flat_14_n.png", "start": 191432437, "end": 191432963}, {"filename": "/GameData/textures/lq_palette/flat_14_o.png", "start": 191432963, "end": 191433489}, {"filename": "/GameData/textures/lq_palette/flat_14_p.png", "start": 191433489, "end": 191434015}, {"filename": "/GameData/textures/lq_palette/flat_15_a_fbr.png", "start": 191434015, "end": 191434541}, {"filename": "/GameData/textures/lq_palette/flat_15_b_fbr.png", "start": 191434541, "end": 191435067}, {"filename": "/GameData/textures/lq_palette/flat_15_c_fbr.png", "start": 191435067, "end": 191435593}, {"filename": "/GameData/textures/lq_palette/flat_15_d_fbr.png", "start": 191435593, "end": 191436119}, {"filename": "/GameData/textures/lq_palette/flat_15_e_fbr.png", "start": 191436119, "end": 191436645}, {"filename": "/GameData/textures/lq_palette/flat_15_f_fbr.png", "start": 191436645, "end": 191437171}, {"filename": "/GameData/textures/lq_palette/flat_15_g_fbr.png", "start": 191437171, "end": 191437697}, {"filename": "/GameData/textures/lq_palette/flat_15_h_fbr.png", "start": 191437697, "end": 191438223}, {"filename": "/GameData/textures/lq_palette/flat_15_i_fbr.png", "start": 191438223, "end": 191438749}, {"filename": "/GameData/textures/lq_palette/flat_15_j_fbr.png", "start": 191438749, "end": 191439275}, {"filename": "/GameData/textures/lq_palette/flat_15_k_fbr.png", "start": 191439275, "end": 191439801}, {"filename": "/GameData/textures/lq_palette/flat_15_l_fbr.png", "start": 191439801, "end": 191440327}, {"filename": "/GameData/textures/lq_palette/flat_15_m_fbr.png", "start": 191440327, "end": 191440853}, {"filename": "/GameData/textures/lq_palette/flat_15_n_fbr.png", "start": 191440853, "end": 191441379}, {"filename": "/GameData/textures/lq_palette/flat_15_o_fbr.png", "start": 191441379, "end": 191441906}, {"filename": "/GameData/textures/lq_palette/flat_15_p_fbr.png", "start": 191441906, "end": 191442433}, {"filename": "/GameData/textures/lq_palette/flat_16_a_fbr.png", "start": 191442433, "end": 191442959}, {"filename": "/GameData/textures/lq_palette/flat_16_b_fbr.png", "start": 191442959, "end": 191443485}, {"filename": "/GameData/textures/lq_palette/flat_16_c_fbr.png", "start": 191443485, "end": 191444011}, {"filename": "/GameData/textures/lq_palette/flat_16_d_fbr.png", "start": 191444011, "end": 191444537}, {"filename": "/GameData/textures/lq_palette/flat_16_e_fbr.png", "start": 191444537, "end": 191445064}, {"filename": "/GameData/textures/lq_palette/flat_16_f_fbr.png", "start": 191445064, "end": 191445591}, {"filename": "/GameData/textures/lq_palette/flat_16_g_fbr.png", "start": 191445591, "end": 191446116}, {"filename": "/GameData/textures/lq_palette/flat_16_h_fbr.png", "start": 191446116, "end": 191446642}, {"filename": "/GameData/textures/lq_palette/flat_16_i_fbr.png", "start": 191446642, "end": 191447168}, {"filename": "/GameData/textures/lq_palette/flat_16_j_fbr.png", "start": 191447168, "end": 191447694}, {"filename": "/GameData/textures/lq_palette/flat_16_k_fbr.png", "start": 191447694, "end": 191448220}, {"filename": "/GameData/textures/lq_palette/flat_16_l_fbr.png", "start": 191448220, "end": 191448746}, {"filename": "/GameData/textures/lq_palette/flat_16_m_fbr.png", "start": 191448746, "end": 191449273}, {"filename": "/GameData/textures/lq_palette/flat_16_n_fbr.png", "start": 191449273, "end": 191449800}, {"filename": "/GameData/textures/lq_palette/flat_16_o_fbr.png", "start": 191449800, "end": 191450325}, {"filename": "/GameData/textures/lq_palette/flat_16_p_fbr.png", "start": 191450325, "end": 191450851}, {"filename": "/GameData/textures/lq_props/JarBod1.png", "start": 191450851, "end": 191451198}, {"filename": "/GameData/textures/lq_props/JarBod2.png", "start": 191451198, "end": 191451516}, {"filename": "/GameData/textures/lq_props/JarTop1.png", "start": 191451516, "end": 191451741}, {"filename": "/GameData/textures/lq_props/JarTop2.png", "start": 191451741, "end": 191451881}, {"filename": "/GameData/textures/lq_props/crate-door-brn.png", "start": 191451881, "end": 191460519}, {"filename": "/GameData/textures/lq_props/crate-door-grn.png", "start": 191460519, "end": 191470575}, {"filename": "/GameData/textures/lq_props/crate-door-orn.png", "start": 191470575, "end": 191479674}, {"filename": "/GameData/textures/lq_props/crate-side-brn.png", "start": 191479674, "end": 191499800}, {"filename": "/GameData/textures/lq_props/crate-side-grn.png", "start": 191499800, "end": 191522713}, {"filename": "/GameData/textures/lq_props/crate-side-orn.png", "start": 191522713, "end": 191544042}, {"filename": "/GameData/textures/lq_props/go-ep0_fbr.png", "start": 191544042, "end": 191545817}, {"filename": "/GameData/textures/lq_props/med_book_blue.png", "start": 191545817, "end": 191546397}, {"filename": "/GameData/textures/lq_props/med_book_green.png", "start": 191546397, "end": 191547020}, {"filename": "/GameData/textures/lq_props/med_book_pink.png", "start": 191547020, "end": 191547675}, {"filename": "/GameData/textures/lq_props/med_book_red.png", "start": 191547675, "end": 191548392}, {"filename": "/GameData/textures/lq_props/med_book_teal.png", "start": 191548392, "end": 191548979}, {"filename": "/GameData/textures/lq_props/med_books_wood.png", "start": 191548979, "end": 191561909}, {"filename": "/GameData/textures/lq_props/med_dbrick4_p1.png", "start": 191561909, "end": 191593746}, {"filename": "/GameData/textures/lq_props/med_dbrick4_p2.png", "start": 191593746, "end": 191627167}, {"filename": "/GameData/textures/lq_props/med_ebrick9_p1.png", "start": 191627167, "end": 191659058}, {"filename": "/GameData/textures/lq_props/med_ebrick9_p2.png", "start": 191659058, "end": 191690424}, {"filename": "/GameData/textures/lq_props/note-e0_fbr.png", "start": 191690424, "end": 191709853}, {"filename": "/GameData/textures/lq_props/plus_0blink_fbr.png", "start": 191709853, "end": 191710082}, {"filename": "/GameData/textures/lq_props/plus_0tvnoise.png", "start": 191710082, "end": 191711068}, {"filename": "/GameData/textures/lq_props/plus_1blink_fbr.png", "start": 191711068, "end": 191711297}, {"filename": "/GameData/textures/lq_props/plus_1tvnoise.png", "start": 191711297, "end": 191712275}, {"filename": "/GameData/textures/lq_props/plus_2blink_fbr.png", "start": 191712275, "end": 191712506}, {"filename": "/GameData/textures/lq_props/plus_2tvnoise.png", "start": 191712506, "end": 191713479}, {"filename": "/GameData/textures/lq_props/plus_3blink_fbr.png", "start": 191713479, "end": 191713710}, {"filename": "/GameData/textures/lq_props/plus_3tvnoise.png", "start": 191713710, "end": 191714692}, {"filename": "/GameData/textures/lq_props/plus_4blink_fbr.png", "start": 191714692, "end": 191714922}, {"filename": "/GameData/textures/lq_props/plus_4tvnoise.png", "start": 191714922, "end": 191715906}, {"filename": "/GameData/textures/lq_props/plus_5tvnoise.png", "start": 191715906, "end": 191716870}, {"filename": "/GameData/textures/lq_props/plus_6tvnoise.png", "start": 191716870, "end": 191717860}, {"filename": "/GameData/textures/lq_props/plus_7tvnoise.png", "start": 191717860, "end": 191718834}, {"filename": "/GameData/textures/lq_props/plus_8tvnoise.png", "start": 191718834, "end": 191719835}, {"filename": "/GameData/textures/lq_props/plus_9tvnoise.png", "start": 191719835, "end": 191720812}, {"filename": "/GameData/textures/lq_props/plus_ablink_fbr.png", "start": 191720812, "end": 191721042}, {"filename": "/GameData/textures/lq_props/plus_atvnoise.png", "start": 191721042, "end": 191721602}, {"filename": "/GameData/textures/lq_props/plus_atvnoise64.png", "start": 191721602, "end": 191722872}, {"filename": "/GameData/textures/lq_props/qr.png", "start": 191722872, "end": 191724292}, {"filename": "/GameData/textures/lq_props/radio16.png", "start": 191724292, "end": 191725060}, {"filename": "/GameData/textures/lq_props/radio32.png", "start": 191725060, "end": 191726050}, {"filename": "/GameData/textures/lq_props/radio64.png", "start": 191726050, "end": 191727808}, {"filename": "/GameData/textures/lq_props/radiowood.png", "start": 191727808, "end": 191729951}, {"filename": "/GameData/textures/lq_props/secret_gem_1.png", "start": 191729951, "end": 191732987}, {"filename": "/GameData/textures/lq_props/secret_gem_2.png", "start": 191732987, "end": 191735691}, {"filename": "/GameData/textures/lq_props/secret_gem_3.png", "start": 191735691, "end": 191738751}, {"filename": "/GameData/textures/lq_props/secret_gem_4.png", "start": 191738751, "end": 191742509}, {"filename": "/GameData/textures/lq_props/secret_gem_h.png", "start": 191742509, "end": 191743060}, {"filename": "/GameData/textures/lq_tech/_t_fence01_fbr.png", "start": 191743060, "end": 191748777}, {"filename": "/GameData/textures/lq_tech/_t_flare01_fbr.png", "start": 191748777, "end": 191749163}, {"filename": "/GameData/textures/lq_tech/aqconc03.png", "start": 191749163, "end": 191759723}, {"filename": "/GameData/textures/lq_tech/aqconc04.png", "start": 191759723, "end": 191770779}, {"filename": "/GameData/textures/lq_tech/aqconc05.png", "start": 191770779, "end": 191775370}, {"filename": "/GameData/textures/lq_tech/aqf006b.png", "start": 191775370, "end": 191777997}, {"filename": "/GameData/textures/lq_tech/aqf032.png", "start": 191777997, "end": 191779268}, {"filename": "/GameData/textures/lq_tech/aqf049.png", "start": 191779268, "end": 191781545}, {"filename": "/GameData/textures/lq_tech/aqf074.png", "start": 191781545, "end": 191784967}, {"filename": "/GameData/textures/lq_tech/aqf075.png", "start": 191784967, "end": 191788126}, {"filename": "/GameData/textures/lq_tech/aqmetl01.png", "start": 191788126, "end": 191790854}, {"filename": "/GameData/textures/lq_tech/aqmetl07.png", "start": 191790854, "end": 191795625}, {"filename": "/GameData/textures/lq_tech/aqmetl14.png", "start": 191795625, "end": 191803357}, {"filename": "/GameData/textures/lq_tech/aqmetl28.png", "start": 191803357, "end": 191805009}, {"filename": "/GameData/textures/lq_tech/aqmetl30.png", "start": 191805009, "end": 191806661}, {"filename": "/GameData/textures/lq_tech/aqmetl33.png", "start": 191806661, "end": 191809097}, {"filename": "/GameData/textures/lq_tech/aqpanl09.png", "start": 191809097, "end": 191811889}, {"filename": "/GameData/textures/lq_tech/aqpanl10.png", "start": 191811889, "end": 191816783}, {"filename": "/GameData/textures/lq_tech/aqpipe01.png", "start": 191816783, "end": 191820971}, {"filename": "/GameData/textures/lq_tech/aqpipe04.png", "start": 191820971, "end": 191823435}, {"filename": "/GameData/textures/lq_tech/aqpipe05.png", "start": 191823435, "end": 191826601}, {"filename": "/GameData/textures/lq_tech/aqpipe08.png", "start": 191826601, "end": 191832233}, {"filename": "/GameData/textures/lq_tech/aqpipe09.png", "start": 191832233, "end": 191840337}, {"filename": "/GameData/textures/lq_tech/aqpipe12.png", "start": 191840337, "end": 191848457}, {"filename": "/GameData/textures/lq_tech/aqpipe13.png", "start": 191848457, "end": 191854768}, {"filename": "/GameData/textures/lq_tech/aqpipe14.png", "start": 191854768, "end": 191862121}, {"filename": "/GameData/textures/lq_tech/aqrust01.png", "start": 191862121, "end": 191864752}, {"filename": "/GameData/textures/lq_tech/aqrust02.png", "start": 191864752, "end": 191870053}, {"filename": "/GameData/textures/lq_tech/aqrust03.png", "start": 191870053, "end": 191875356}, {"filename": "/GameData/textures/lq_tech/aqrust03b.png", "start": 191875356, "end": 191878086}, {"filename": "/GameData/textures/lq_tech/aqrust04.png", "start": 191878086, "end": 191879479}, {"filename": "/GameData/textures/lq_tech/aqrust09.png", "start": 191879479, "end": 191883367}, {"filename": "/GameData/textures/lq_tech/aqrust10.png", "start": 191883367, "end": 191886533}, {"filename": "/GameData/textures/lq_tech/aqsect14.png", "start": 191886533, "end": 191888909}, {"filename": "/GameData/textures/lq_tech/aqsect15.png", "start": 191888909, "end": 191891856}, {"filename": "/GameData/textures/lq_tech/aqsect16.png", "start": 191891856, "end": 191895102}, {"filename": "/GameData/textures/lq_tech/aqsect16b.png", "start": 191895102, "end": 191900059}, {"filename": "/GameData/textures/lq_tech/aqsupp01.png", "start": 191900059, "end": 191902705}, {"filename": "/GameData/textures/lq_tech/aqsupp02.png", "start": 191902705, "end": 191907570}, {"filename": "/GameData/textures/lq_tech/aqsupp03.png", "start": 191907570, "end": 191911472}, {"filename": "/GameData/textures/lq_tech/aqsupp04.png", "start": 191911472, "end": 191913015}, {"filename": "/GameData/textures/lq_tech/aqsupp06.png", "start": 191913015, "end": 191914394}, {"filename": "/GameData/textures/lq_tech/aqsupp07.png", "start": 191914394, "end": 191915749}, {"filename": "/GameData/textures/lq_tech/aqsupp08.png", "start": 191915749, "end": 191917951}, {"filename": "/GameData/textures/lq_tech/aqsupp09.png", "start": 191917951, "end": 191920804}, {"filename": "/GameData/textures/lq_tech/aqtrim01.png", "start": 191920804, "end": 191922031}, {"filename": "/GameData/textures/lq_tech/aqtrim02.png", "start": 191922031, "end": 191922955}, {"filename": "/GameData/textures/lq_tech/aqtrim03.png", "start": 191922955, "end": 191923526}, {"filename": "/GameData/textures/lq_tech/aqtrim08.png", "start": 191923526, "end": 191924258}, {"filename": "/GameData/textures/lq_tech/butmet.png", "start": 191924258, "end": 191926169}, {"filename": "/GameData/textures/lq_tech/comp1_1.png", "start": 191926169, "end": 191929525}, {"filename": "/GameData/textures/lq_tech/comp1_2.png", "start": 191929525, "end": 191932204}, {"filename": "/GameData/textures/lq_tech/comp1_3.png", "start": 191932204, "end": 191934693}, {"filename": "/GameData/textures/lq_tech/comp1_3b.png", "start": 191934693, "end": 191937321}, {"filename": "/GameData/textures/lq_tech/comp1_4.png", "start": 191937321, "end": 191938926}, {"filename": "/GameData/textures/lq_tech/comp1_5.png", "start": 191938926, "end": 191942025}, {"filename": "/GameData/textures/lq_tech/comp1_6.png", "start": 191942025, "end": 191945467}, {"filename": "/GameData/textures/lq_tech/comp1_7.png", "start": 191945467, "end": 191947812}, {"filename": "/GameData/textures/lq_tech/comp1_8.png", "start": 191947812, "end": 191950141}, {"filename": "/GameData/textures/lq_tech/compbase.png", "start": 191950141, "end": 191952560}, {"filename": "/GameData/textures/lq_tech/crate.png", "start": 191952560, "end": 191955677}, {"filename": "/GameData/textures/lq_tech/crate0_bottom.png", "start": 191955677, "end": 191957793}, {"filename": "/GameData/textures/lq_tech/crate0_s_bottom.png", "start": 191957793, "end": 191958881}, {"filename": "/GameData/textures/lq_tech/crate0_s_sside.png", "start": 191958881, "end": 191959826}, {"filename": "/GameData/textures/lq_tech/crate0_s_top.png", "start": 191959826, "end": 191961265}, {"filename": "/GameData/textures/lq_tech/crate0_s_tside.png", "start": 191961265, "end": 191962928}, {"filename": "/GameData/textures/lq_tech/crate0_side.png", "start": 191962928, "end": 191966396}, {"filename": "/GameData/textures/lq_tech/crate0_top.png", "start": 191966396, "end": 191969142}, {"filename": "/GameData/textures/lq_tech/crate0_xs_bot.png", "start": 191969142, "end": 191969809}, {"filename": "/GameData/textures/lq_tech/crate0_xs_sside.png", "start": 191969809, "end": 191970754}, {"filename": "/GameData/textures/lq_tech/crate0_xs_top.png", "start": 191970754, "end": 191971624}, {"filename": "/GameData/textures/lq_tech/crate0_xs_tside.png", "start": 191971624, "end": 191972564}, {"filename": "/GameData/textures/lq_tech/crate1_bottom.png", "start": 191972564, "end": 191974528}, {"filename": "/GameData/textures/lq_tech/crate1_s_bottom.png", "start": 191974528, "end": 191975558}, {"filename": "/GameData/textures/lq_tech/crate1_s_sside.png", "start": 191975558, "end": 191976436}, {"filename": "/GameData/textures/lq_tech/crate1_s_top.png", "start": 191976436, "end": 191977569}, {"filename": "/GameData/textures/lq_tech/crate1_s_tside.png", "start": 191977569, "end": 191979034}, {"filename": "/GameData/textures/lq_tech/crate1_side.png", "start": 191979034, "end": 191982151}, {"filename": "/GameData/textures/lq_tech/crate1_top.png", "start": 191982151, "end": 191984272}, {"filename": "/GameData/textures/lq_tech/crate1_xs_bot.png", "start": 191984272, "end": 191984926}, {"filename": "/GameData/textures/lq_tech/crate1_xs_sside.png", "start": 191984926, "end": 191985804}, {"filename": "/GameData/textures/lq_tech/crate1_xs_top.png", "start": 191985804, "end": 191986511}, {"filename": "/GameData/textures/lq_tech/crate1_xs_tside.png", "start": 191986511, "end": 191987349}, {"filename": "/GameData/textures/lq_tech/dem4_1.png", "start": 191987349, "end": 191996748}, {"filename": "/GameData/textures/lq_tech/dem4_4.png", "start": 191996748, "end": 192004794}, {"filename": "/GameData/textures/lq_tech/dem5_3_fbr.png", "start": 192004794, "end": 192013611}, {"filename": "/GameData/textures/lq_tech/door02_1.png", "start": 192013611, "end": 192017743}, {"filename": "/GameData/textures/lq_tech/doorr02_1.png", "start": 192017743, "end": 192019309}, {"filename": "/GameData/textures/lq_tech/doortrak1.png", "start": 192019309, "end": 192020920}, {"filename": "/GameData/textures/lq_tech/doortrak2-corn.png", "start": 192020920, "end": 192022217}, {"filename": "/GameData/textures/lq_tech/doortrak2.png", "start": 192022217, "end": 192023882}, {"filename": "/GameData/textures/lq_tech/ecop1_1.png", "start": 192023882, "end": 192026959}, {"filename": "/GameData/textures/lq_tech/ecop1_4.png", "start": 192026959, "end": 192030560}, {"filename": "/GameData/textures/lq_tech/edoor01_1.png", "start": 192030560, "end": 192042706}, {"filename": "/GameData/textures/lq_tech/edoor02.png", "start": 192042706, "end": 192054757}, {"filename": "/GameData/textures/lq_tech/edoor02.png.png", "start": 192054757, "end": 192066808}, {"filename": "/GameData/textures/lq_tech/fddoor01.png", "start": 192066808, "end": 192075562}, {"filename": "/GameData/textures/lq_tech/fddoor01b.png", "start": 192075562, "end": 192086245}, {"filename": "/GameData/textures/lq_tech/fdoor02.png", "start": 192086245, "end": 192094799}, {"filename": "/GameData/textures/lq_tech/flat4.png", "start": 192094799, "end": 192095972}, {"filename": "/GameData/textures/lq_tech/floor5_2.png", "start": 192095972, "end": 192098698}, {"filename": "/GameData/textures/lq_tech/floor5_3.png", "start": 192098698, "end": 192100878}, {"filename": "/GameData/textures/lq_tech/laserfield1_fbr.png", "start": 192100878, "end": 192111887}, {"filename": "/GameData/textures/lq_tech/light2.png", "start": 192111887, "end": 192112485}, {"filename": "/GameData/textures/lq_tech/lit8nb.png", "start": 192112485, "end": 192112799}, {"filename": "/GameData/textures/lq_tech/lit8sfb_fbr.png", "start": 192112799, "end": 192113096}, {"filename": "/GameData/textures/lq_tech/met2.png", "start": 192113096, "end": 192125420}, {"filename": "/GameData/textures/lq_tech/metalstrip_1.png", "start": 192125420, "end": 192127703}, {"filename": "/GameData/textures/lq_tech/plat_side1.png", "start": 192127703, "end": 192128468}, {"filename": "/GameData/textures/lq_tech/plat_stem.png", "start": 192128468, "end": 192129114}, {"filename": "/GameData/textures/lq_tech/plat_top1.png", "start": 192129114, "end": 192132514}, {"filename": "/GameData/textures/lq_tech/plat_top2.png", "start": 192132514, "end": 192136229}, {"filename": "/GameData/textures/lq_tech/plat_top3.png", "start": 192136229, "end": 192139637}, {"filename": "/GameData/textures/lq_tech/plat_top4.png", "start": 192139637, "end": 192142333}, {"filename": "/GameData/textures/lq_tech/plat_top5.png", "start": 192142333, "end": 192144428}, {"filename": "/GameData/textures/lq_tech/plus_0_gkey.png", "start": 192144428, "end": 192145492}, {"filename": "/GameData/textures/lq_tech/plus_0_skey.png", "start": 192145492, "end": 192146508}, {"filename": "/GameData/textures/lq_tech/plus_0_tscrn0.png", "start": 192146508, "end": 192148621}, {"filename": "/GameData/textures/lq_tech/plus_0_tscrn1.png", "start": 192148621, "end": 192150746}, {"filename": "/GameData/textures/lq_tech/plus_0arrow2_d_fbr.png", "start": 192150746, "end": 192152000}, {"filename": "/GameData/textures/lq_tech/plus_0arrow2_h_fbr.png", "start": 192152000, "end": 192153269}, {"filename": "/GameData/textures/lq_tech/plus_0arrow2_u_fbr.png", "start": 192153269, "end": 192154527}, {"filename": "/GameData/textures/lq_tech/plus_0arrow_d_fbr.png", "start": 192154527, "end": 192155409}, {"filename": "/GameData/textures/lq_tech/plus_0arrow_h_fbr.png", "start": 192155409, "end": 192156288}, {"filename": "/GameData/textures/lq_tech/plus_0arrow_u_fbr.png", "start": 192156288, "end": 192157163}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn1b_fbr.png", "start": 192157163, "end": 192158052}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn2_fbr.png", "start": 192158052, "end": 192158398}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn2b_fbr.png", "start": 192158398, "end": 192158738}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn_fbr.png", "start": 192158738, "end": 192160019}, {"filename": "/GameData/textures/lq_tech/plus_0button3_fbr.png", "start": 192160019, "end": 192161895}, {"filename": "/GameData/textures/lq_tech/plus_0lit8s.png", "start": 192161895, "end": 192162209}, {"filename": "/GameData/textures/lq_tech/plus_0planet_a_fbr.png", "start": 192162209, "end": 192163800}, {"filename": "/GameData/textures/lq_tech/plus_0planet_b_fbr.png", "start": 192163800, "end": 192164910}, {"filename": "/GameData/textures/lq_tech/plus_0planet_c_fbr.png", "start": 192164910, "end": 192166010}, {"filename": "/GameData/textures/lq_tech/plus_0slipbot.png", "start": 192166010, "end": 192169293}, {"filename": "/GameData/textures/lq_tech/plus_0sliptop.png", "start": 192169293, "end": 192172936}, {"filename": "/GameData/textures/lq_tech/plus_0tek_jump1_fbr.png", "start": 192172936, "end": 192175419}, {"filename": "/GameData/textures/lq_tech/plus_0term128.png", "start": 192175419, "end": 192178223}, {"filename": "/GameData/textures/lq_tech/plus_0term64.png", "start": 192178223, "end": 192179415}, {"filename": "/GameData/textures/lq_tech/plus_0tlight1.png", "start": 192179415, "end": 192180013}, {"filename": "/GameData/textures/lq_tech/plus_0tlight2.png", "start": 192180013, "end": 192180626}, {"filename": "/GameData/textures/lq_tech/plus_0tlight3.png", "start": 192180626, "end": 192181197}, {"filename": "/GameData/textures/lq_tech/plus_1_gkey.png", "start": 192181197, "end": 192182247}, {"filename": "/GameData/textures/lq_tech/plus_1_skey.png", "start": 192182247, "end": 192183252}, {"filename": "/GameData/textures/lq_tech/plus_1arrow2_d_fbr.png", "start": 192183252, "end": 192184508}, {"filename": "/GameData/textures/lq_tech/plus_1arrow2_h_fbr.png", "start": 192184508, "end": 192185775}, {"filename": "/GameData/textures/lq_tech/plus_1arrow2_u_fbr.png", "start": 192185775, "end": 192187033}, {"filename": "/GameData/textures/lq_tech/plus_1arrow_d_fbr.png", "start": 192187033, "end": 192187919}, {"filename": "/GameData/textures/lq_tech/plus_1arrow_h_fbr.png", "start": 192187919, "end": 192188801}, {"filename": "/GameData/textures/lq_tech/plus_1arrow_u_fbr.png", "start": 192188801, "end": 192189679}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn.png", "start": 192189679, "end": 192190958}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn1b.png", "start": 192190958, "end": 192191836}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn2.png", "start": 192191836, "end": 192192186}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn2b.png", "start": 192192186, "end": 192192544}, {"filename": "/GameData/textures/lq_tech/plus_1planet_a_fbr.png", "start": 192192544, "end": 192194165}, {"filename": "/GameData/textures/lq_tech/plus_1planet_b_fbr.png", "start": 192194165, "end": 192195250}, {"filename": "/GameData/textures/lq_tech/plus_1planet_c_fbr.png", "start": 192195250, "end": 192196322}, {"filename": "/GameData/textures/lq_tech/plus_1tek_jump1_fbr.png", "start": 192196322, "end": 192198805}, {"filename": "/GameData/textures/lq_tech/plus_1term128.png", "start": 192198805, "end": 192201608}, {"filename": "/GameData/textures/lq_tech/plus_1term64.png", "start": 192201608, "end": 192202800}, {"filename": "/GameData/textures/lq_tech/plus_2_gkey.png", "start": 192202800, "end": 192203862}, {"filename": "/GameData/textures/lq_tech/plus_2_skey.png", "start": 192203862, "end": 192204868}, {"filename": "/GameData/textures/lq_tech/plus_2arrow2_d_fbr.png", "start": 192204868, "end": 192206111}, {"filename": "/GameData/textures/lq_tech/plus_2arrow2_h_fbr.png", "start": 192206111, "end": 192207372}, {"filename": "/GameData/textures/lq_tech/plus_2arrow2_u_fbr.png", "start": 192207372, "end": 192208618}, {"filename": "/GameData/textures/lq_tech/plus_2arrow_d_fbr.png", "start": 192208618, "end": 192209516}, {"filename": "/GameData/textures/lq_tech/plus_2arrow_h_fbr.png", "start": 192209516, "end": 192210406}, {"filename": "/GameData/textures/lq_tech/plus_2arrow_u_fbr.png", "start": 192210406, "end": 192211293}, {"filename": "/GameData/textures/lq_tech/plus_2planet_a_fbr.png", "start": 192211293, "end": 192212896}, {"filename": "/GameData/textures/lq_tech/plus_2planet_b_fbr.png", "start": 192212896, "end": 192213998}, {"filename": "/GameData/textures/lq_tech/plus_2planet_c_fbr.png", "start": 192213998, "end": 192215116}, {"filename": "/GameData/textures/lq_tech/plus_3planet_a_fbr.png", "start": 192215116, "end": 192216706}, {"filename": "/GameData/textures/lq_tech/plus_3planet_b_fbr.png", "start": 192216706, "end": 192217800}, {"filename": "/GameData/textures/lq_tech/plus_3planet_c_fbr.png", "start": 192217800, "end": 192218923}, {"filename": "/GameData/textures/lq_tech/plus_4planet_a_fbr.png", "start": 192218923, "end": 192220532}, {"filename": "/GameData/textures/lq_tech/plus_4planet_b_fbr.png", "start": 192220532, "end": 192221647}, {"filename": "/GameData/textures/lq_tech/plus_4planet_c_fbr.png", "start": 192221647, "end": 192222794}, {"filename": "/GameData/textures/lq_tech/plus_5planet_a_fbr.png", "start": 192222794, "end": 192224426}, {"filename": "/GameData/textures/lq_tech/plus_5planet_b_fbr.png", "start": 192224426, "end": 192225553}, {"filename": "/GameData/textures/lq_tech/plus_5planet_c_fbr.png", "start": 192225553, "end": 192226660}, {"filename": "/GameData/textures/lq_tech/plus_6planet_a_fbr.png", "start": 192226660, "end": 192228281}, {"filename": "/GameData/textures/lq_tech/plus_6planet_b_fbr.png", "start": 192228281, "end": 192229405}, {"filename": "/GameData/textures/lq_tech/plus_6planet_c_fbr.png", "start": 192229405, "end": 192230499}, {"filename": "/GameData/textures/lq_tech/plus_7planet_a_fbr.png", "start": 192230499, "end": 192232087}, {"filename": "/GameData/textures/lq_tech/plus_7planet_b_fbr.png", "start": 192232087, "end": 192233200}, {"filename": "/GameData/textures/lq_tech/plus_7planet_c_fbr.png", "start": 192233200, "end": 192234292}, {"filename": "/GameData/textures/lq_tech/plus_8planet_a_fbr.png", "start": 192234292, "end": 192235914}, {"filename": "/GameData/textures/lq_tech/plus_8planet_b_fbr.png", "start": 192235914, "end": 192237041}, {"filename": "/GameData/textures/lq_tech/plus_8planet_c_fbr.png", "start": 192237041, "end": 192238130}, {"filename": "/GameData/textures/lq_tech/plus_9planet_a_fbr.png", "start": 192238130, "end": 192239714}, {"filename": "/GameData/textures/lq_tech/plus_9planet_b_fbr.png", "start": 192239714, "end": 192240802}, {"filename": "/GameData/textures/lq_tech/plus_9planet_c_fbr.png", "start": 192240802, "end": 192241878}, {"filename": "/GameData/textures/lq_tech/plus_A_tscrn0.png", "start": 192241878, "end": 192243501}, {"filename": "/GameData/textures/lq_tech/plus_A_tscrn1.png", "start": 192243501, "end": 192246010}, {"filename": "/GameData/textures/lq_tech/plus_A_tscrn2.png", "start": 192246010, "end": 192247610}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn.png", "start": 192247610, "end": 192248899}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn1b.png", "start": 192248899, "end": 192250188}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn2.png", "start": 192250188, "end": 192250559}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn2b.png", "start": 192250559, "end": 192250930}, {"filename": "/GameData/textures/lq_tech/plus_abasebtnb.png", "start": 192250930, "end": 192251301}, {"filename": "/GameData/textures/lq_tech/plus_abutton3_fbr.png", "start": 192251301, "end": 192253189}, {"filename": "/GameData/textures/lq_tech/plus_alit8s_fbr.png", "start": 192253189, "end": 192253486}, {"filename": "/GameData/textures/lq_tech/plus_atek_jump1_fbr.png", "start": 192253486, "end": 192255979}, {"filename": "/GameData/textures/lq_tech/plus_atlight1_fbr.png", "start": 192255979, "end": 192256569}, {"filename": "/GameData/textures/lq_tech/plus_atlight2_fbr.png", "start": 192256569, "end": 192257142}, {"filename": "/GameData/textures/lq_tech/plus_atlight3_fbr.png", "start": 192257142, "end": 192257744}, {"filename": "/GameData/textures/lq_tech/rw33_1.png", "start": 192257744, "end": 192263309}, {"filename": "/GameData/textures/lq_tech/rw33_2.png", "start": 192263309, "end": 192269163}, {"filename": "/GameData/textures/lq_tech/rw33_3.png", "start": 192269163, "end": 192274909}, {"filename": "/GameData/textures/lq_tech/rw33_4.png", "start": 192274909, "end": 192277475}, {"filename": "/GameData/textures/lq_tech/rw33_4b_l.png", "start": 192277475, "end": 192280435}, {"filename": "/GameData/textures/lq_tech/rw33_5.png", "start": 192280435, "end": 192283307}, {"filename": "/GameData/textures/lq_tech/rw33_flat.png", "start": 192283307, "end": 192289258}, {"filename": "/GameData/textures/lq_tech/rw33_lit.png", "start": 192289258, "end": 192290082}, {"filename": "/GameData/textures/lq_tech/rw33b_1.png", "start": 192290082, "end": 192295545}, {"filename": "/GameData/textures/lq_tech/rw33b_2.png", "start": 192295545, "end": 192301387}, {"filename": "/GameData/textures/lq_tech/rw33b_3.png", "start": 192301387, "end": 192307374}, {"filename": "/GameData/textures/lq_tech/rw33b_4.png", "start": 192307374, "end": 192310136}, {"filename": "/GameData/textures/lq_tech/rw33b_5.png", "start": 192310136, "end": 192313185}, {"filename": "/GameData/textures/lq_tech/rw33b_flat.png", "start": 192313185, "end": 192319383}, {"filename": "/GameData/textures/lq_tech/rw33b_lit.png", "start": 192319383, "end": 192320335}, {"filename": "/GameData/textures/lq_tech/rw37_1.png", "start": 192320335, "end": 192325947}, {"filename": "/GameData/textures/lq_tech/rw37_2.png", "start": 192325947, "end": 192332077}, {"filename": "/GameData/textures/lq_tech/rw37_3.png", "start": 192332077, "end": 192337972}, {"filename": "/GameData/textures/lq_tech/rw37_4.png", "start": 192337972, "end": 192344983}, {"filename": "/GameData/textures/lq_tech/rw37_trim1.png", "start": 192344983, "end": 192347491}, {"filename": "/GameData/textures/lq_tech/rw37_trim2.png", "start": 192347491, "end": 192349274}, {"filename": "/GameData/textures/lq_tech/rw37_trim3.png", "start": 192349274, "end": 192351765}, {"filename": "/GameData/textures/lq_tech/rw39_1_fbr.png", "start": 192351765, "end": 192357710}, {"filename": "/GameData/textures/lq_tech/spotlight_fbr.png", "start": 192357710, "end": 192360322}, {"filename": "/GameData/textures/lq_tech/star_lasergrid.png", "start": 192360322, "end": 192360702}, {"filename": "/GameData/textures/lq_tech/t_band1a.png", "start": 192360702, "end": 192363206}, {"filename": "/GameData/textures/lq_tech/t_band1b.png", "start": 192363206, "end": 192365814}, {"filename": "/GameData/textures/lq_tech/t_blok01.png", "start": 192365814, "end": 192368492}, {"filename": "/GameData/textures/lq_tech/t_blok01a.png", "start": 192368492, "end": 192371264}, {"filename": "/GameData/textures/lq_tech/t_blok02.png", "start": 192371264, "end": 192376391}, {"filename": "/GameData/textures/lq_tech/t_blok02a.png", "start": 192376391, "end": 192381250}, {"filename": "/GameData/textures/lq_tech/t_blok03.png", "start": 192381250, "end": 192383692}, {"filename": "/GameData/textures/lq_tech/t_blok03a.png", "start": 192383692, "end": 192385547}, {"filename": "/GameData/textures/lq_tech/t_blok04.png", "start": 192385547, "end": 192388613}, {"filename": "/GameData/textures/lq_tech/t_blok04h.png", "start": 192388613, "end": 192391337}, {"filename": "/GameData/textures/lq_tech/t_blok05.png", "start": 192391337, "end": 192395625}, {"filename": "/GameData/textures/lq_tech/t_blok06.png", "start": 192395625, "end": 192398475}, {"filename": "/GameData/textures/lq_tech/t_blok06h.png", "start": 192398475, "end": 192400452}, {"filename": "/GameData/textures/lq_tech/t_blok07.png", "start": 192400452, "end": 192403238}, {"filename": "/GameData/textures/lq_tech/t_blok07a.png", "start": 192403238, "end": 192406024}, {"filename": "/GameData/textures/lq_tech/t_blok08.png", "start": 192406024, "end": 192410531}, {"filename": "/GameData/textures/lq_tech/t_blok09.png", "start": 192410531, "end": 192413851}, {"filename": "/GameData/textures/lq_tech/t_blok10.png", "start": 192413851, "end": 192419862}, {"filename": "/GameData/textures/lq_tech/t_blok10b.png", "start": 192419862, "end": 192425570}, {"filename": "/GameData/textures/lq_tech/t_blok10c.png", "start": 192425570, "end": 192428788}, {"filename": "/GameData/textures/lq_tech/t_blok11.png", "start": 192428788, "end": 192434729}, {"filename": "/GameData/textures/lq_tech/t_blok11b.png", "start": 192434729, "end": 192440334}, {"filename": "/GameData/textures/lq_tech/t_blok12c.png", "start": 192440334, "end": 192443541}, {"filename": "/GameData/textures/lq_tech/t_flat01.png", "start": 192443541, "end": 192445975}, {"filename": "/GameData/textures/lq_tech/t_flat02.png", "start": 192445975, "end": 192448340}, {"filename": "/GameData/textures/lq_tech/t_flat05.png", "start": 192448340, "end": 192456536}, {"filename": "/GameData/textures/lq_tech/t_flor1a.png", "start": 192456536, "end": 192460148}, {"filename": "/GameData/textures/lq_tech/t_flor1b.png", "start": 192460148, "end": 192463766}, {"filename": "/GameData/textures/lq_tech/t_flor2a.png", "start": 192463766, "end": 192466360}, {"filename": "/GameData/textures/lq_tech/t_flor2b.png", "start": 192466360, "end": 192468296}, {"filename": "/GameData/textures/lq_tech/t_flor2c.png", "start": 192468296, "end": 192469776}, {"filename": "/GameData/textures/lq_tech/t_flor2d.png", "start": 192469776, "end": 192470280}, {"filename": "/GameData/textures/lq_tech/t_lit01_fbr.png", "start": 192470280, "end": 192470460}, {"filename": "/GameData/textures/lq_tech/t_lit02_fbr.png", "start": 192470460, "end": 192470602}, {"filename": "/GameData/textures/lq_tech/t_lit03_fbr.png", "start": 192470602, "end": 192470737}, {"filename": "/GameData/textures/lq_tech/t_lit04_fbr.png", "start": 192470737, "end": 192470869}, {"filename": "/GameData/textures/lq_tech/t_lit05_fbr.png", "start": 192470869, "end": 192471035}, {"filename": "/GameData/textures/lq_tech/t_lit06_fbr.png", "start": 192471035, "end": 192471191}, {"filename": "/GameData/textures/lq_tech/t_lit07_fbr.png", "start": 192471191, "end": 192473494}, {"filename": "/GameData/textures/lq_tech/t_lit08_fbr.png", "start": 192473494, "end": 192473711}, {"filename": "/GameData/textures/lq_tech/t_metalsheeta.png", "start": 192473711, "end": 192480310}, {"filename": "/GameData/textures/lq_tech/t_metalsheetb.png", "start": 192480310, "end": 192491823}, {"filename": "/GameData/textures/lq_tech/t_num_0_fbr.png", "start": 192491823, "end": 192492597}, {"filename": "/GameData/textures/lq_tech/t_num_1_fbr.png", "start": 192492597, "end": 192493359}, {"filename": "/GameData/textures/lq_tech/t_num_2_fbr.png", "start": 192493359, "end": 192494154}, {"filename": "/GameData/textures/lq_tech/t_num_3_fbr.png", "start": 192494154, "end": 192494912}, {"filename": "/GameData/textures/lq_tech/t_num_4_fbr.png", "start": 192494912, "end": 192495717}, {"filename": "/GameData/textures/lq_tech/t_num_5_fbr.png", "start": 192495717, "end": 192496515}, {"filename": "/GameData/textures/lq_tech/t_num_6_fbr.png", "start": 192496515, "end": 192497312}, {"filename": "/GameData/textures/lq_tech/t_num_7_fbr.png", "start": 192497312, "end": 192498086}, {"filename": "/GameData/textures/lq_tech/t_num_8_fbr.png", "start": 192498086, "end": 192498866}, {"filename": "/GameData/textures/lq_tech/t_num_9_fbr.png", "start": 192498866, "end": 192499659}, {"filename": "/GameData/textures/lq_tech/t_num_x.png", "start": 192499659, "end": 192500467}, {"filename": "/GameData/textures/lq_tech/t_rivs01.png", "start": 192500467, "end": 192502895}, {"filename": "/GameData/textures/lq_tech/t_rivs01a.png", "start": 192502895, "end": 192505349}, {"filename": "/GameData/textures/lq_tech/t_sign1.png", "start": 192505349, "end": 192508299}, {"filename": "/GameData/textures/lq_tech/t_tech01.png", "start": 192508299, "end": 192512350}, {"filename": "/GameData/textures/lq_tech/t_tech02.png", "start": 192512350, "end": 192515453}, {"filename": "/GameData/textures/lq_tech/t_tech03.png", "start": 192515453, "end": 192531444}, {"filename": "/GameData/textures/lq_tech/t_tech04.png", "start": 192531444, "end": 192534299}, {"filename": "/GameData/textures/lq_tech/t_tech05.png", "start": 192534299, "end": 192537078}, {"filename": "/GameData/textures/lq_tech/t_tech06.png", "start": 192537078, "end": 192539890}, {"filename": "/GameData/textures/lq_tech/t_trim1a.png", "start": 192539890, "end": 192542378}, {"filename": "/GameData/textures/lq_tech/t_trim1aa.png", "start": 192542378, "end": 192545013}, {"filename": "/GameData/textures/lq_tech/t_trim1b.png", "start": 192545013, "end": 192547744}, {"filename": "/GameData/textures/lq_tech/t_trim1ba.png", "start": 192547744, "end": 192550368}, {"filename": "/GameData/textures/lq_tech/t_trim1c.png", "start": 192550368, "end": 192553153}, {"filename": "/GameData/textures/lq_tech/t_trim1ca.png", "start": 192553153, "end": 192555979}, {"filename": "/GameData/textures/lq_tech/t_trim1d.png", "start": 192555979, "end": 192558279}, {"filename": "/GameData/textures/lq_tech/t_trim1e.png", "start": 192558279, "end": 192560424}, {"filename": "/GameData/textures/lq_tech/t_trim2a.png", "start": 192560424, "end": 192563497}, {"filename": "/GameData/textures/lq_tech/t_trim2aa.png", "start": 192563497, "end": 192566149}, {"filename": "/GameData/textures/lq_tech/t_trim2b.png", "start": 192566149, "end": 192568777}, {"filename": "/GameData/textures/lq_tech/t_trim2ba.png", "start": 192568777, "end": 192571465}, {"filename": "/GameData/textures/lq_tech/t_trim2c.png", "start": 192571465, "end": 192574256}, {"filename": "/GameData/textures/lq_tech/t_trim2ca.png", "start": 192574256, "end": 192577135}, {"filename": "/GameData/textures/lq_tech/t_trim2d.png", "start": 192577135, "end": 192579597}, {"filename": "/GameData/textures/lq_tech/t_trim2e.png", "start": 192579597, "end": 192582104}, {"filename": "/GameData/textures/lq_tech/t_tris02.png", "start": 192582104, "end": 192585018}, {"filename": "/GameData/textures/lq_tech/t_wall05.png", "start": 192585018, "end": 192588344}, {"filename": "/GameData/textures/lq_tech/t_wall1a.png", "start": 192588344, "end": 192598871}, {"filename": "/GameData/textures/lq_tech/t_wall1aa.png", "start": 192598871, "end": 192609192}, {"filename": "/GameData/textures/lq_tech/t_wall1b.png", "start": 192609192, "end": 192620292}, {"filename": "/GameData/textures/lq_tech/t_wall1ba.png", "start": 192620292, "end": 192631131}, {"filename": "/GameData/textures/lq_tech/t_wall2a.png", "start": 192631131, "end": 192641915}, {"filename": "/GameData/textures/lq_tech/t_wall2aa.png", "start": 192641915, "end": 192652935}, {"filename": "/GameData/textures/lq_tech/t_wall2ab.png", "start": 192652935, "end": 192664692}, {"filename": "/GameData/textures/lq_tech/t_wall2b.png", "start": 192664692, "end": 192678930}, {"filename": "/GameData/textures/lq_tech/t_wall2ba.png", "start": 192678930, "end": 192692381}, {"filename": "/GameData/textures/lq_tech/t_wall3a.png", "start": 192692381, "end": 192700348}, {"filename": "/GameData/textures/lq_tech/t_wall3aa.png", "start": 192700348, "end": 192708615}, {"filename": "/GameData/textures/lq_tech/t_wall3b.png", "start": 192708615, "end": 192716937}, {"filename": "/GameData/textures/lq_tech/t_wall3ba.png", "start": 192716937, "end": 192725900}, {"filename": "/GameData/textures/lq_tech/t_wall6a.png", "start": 192725900, "end": 192729134}, {"filename": "/GameData/textures/lq_tech/t_wall6b.png", "start": 192729134, "end": 192732148}, {"filename": "/GameData/textures/lq_tech/t_wall6c.png", "start": 192732148, "end": 192735824}, {"filename": "/GameData/textures/lq_tech/t_wall6d.png", "start": 192735824, "end": 192739527}, {"filename": "/GameData/textures/lq_tech/t_wall6e.png", "start": 192739527, "end": 192743323}, {"filename": "/GameData/textures/lq_tech/t_wall7a.png", "start": 192743323, "end": 192754215}, {"filename": "/GameData/textures/lq_tech/t_wall7b.png", "start": 192754215, "end": 192761656}, {"filename": "/GameData/textures/lq_tech/t_wire01.png", "start": 192761656, "end": 192764592}, {"filename": "/GameData/textures/lq_tech/t_wire02.png", "start": 192764592, "end": 192768127}, {"filename": "/GameData/textures/lq_tech/t_wire03.png", "start": 192768127, "end": 192771727}, {"filename": "/GameData/textures/lq_tech/tech04_1.png", "start": 192771727, "end": 192772550}, {"filename": "/GameData/textures/lq_tech/tech04_3.png", "start": 192772550, "end": 192774019}, {"filename": "/GameData/textures/lq_tech/tech08_1.png", "start": 192774019, "end": 192785072}, {"filename": "/GameData/textures/lq_tech/tech08_2.png", "start": 192785072, "end": 192796125}, {"filename": "/GameData/textures/lq_tech/tech10_3.png", "start": 192796125, "end": 192799981}, {"filename": "/GameData/textures/lq_tech/tech14-1.png", "start": 192799981, "end": 192810252}, {"filename": "/GameData/textures/lq_tech/techbasetextures.txt", "start": 192810252, "end": 192810809}, {"filename": "/GameData/textures/lq_tech/techeye1_fbr.png", "start": 192810809, "end": 192814153}, {"filename": "/GameData/textures/lq_tech/techeye2_fbr.png", "start": 192814153, "end": 192817555}, {"filename": "/GameData/textures/lq_tech/tek_door1.png", "start": 192817555, "end": 192829701}, {"filename": "/GameData/textures/lq_tech/tek_door2.png", "start": 192829701, "end": 192841752}, {"filename": "/GameData/textures/lq_tech/tek_flr3.png", "start": 192841752, "end": 192845133}, {"filename": "/GameData/textures/lq_tech/tek_grate.png", "start": 192845133, "end": 192848011}, {"filename": "/GameData/textures/lq_tech/tek_lit1_fbr.png", "start": 192848011, "end": 192849646}, {"filename": "/GameData/textures/lq_tech/tek_lit2_fbr.png", "start": 192849646, "end": 192850632}, {"filename": "/GameData/textures/lq_tech/tek_lit3_fbr.png", "start": 192850632, "end": 192852474}, {"filename": "/GameData/textures/lq_tech/tek_lit4_fbr.png", "start": 192852474, "end": 192853601}, {"filename": "/GameData/textures/lq_tech/tek_pip1_fbr.png", "start": 192853601, "end": 192856683}, {"filename": "/GameData/textures/lq_tech/tek_pipe1.png", "start": 192856683, "end": 192859483}, {"filename": "/GameData/textures/lq_tech/tek_pipe2.png", "start": 192859483, "end": 192861142}, {"filename": "/GameData/textures/lq_tech/tek_trm1.png", "start": 192861142, "end": 192863600}, {"filename": "/GameData/textures/lq_tech/tek_trm3.png", "start": 192863600, "end": 192866210}, {"filename": "/GameData/textures/lq_tech/tek_wall4_1.png", "start": 192866210, "end": 192880655}, {"filename": "/GameData/textures/lq_tech/tele_frame1.png", "start": 192880655, "end": 192886810}, {"filename": "/GameData/textures/lq_tech/tele_frame2.png", "start": 192886810, "end": 192888761}, {"filename": "/GameData/textures/lq_tech/tele_frame3.png", "start": 192888761, "end": 192892748}, {"filename": "/GameData/textures/lq_tech/telepad1_fbr.png", "start": 192892748, "end": 192894724}, {"filename": "/GameData/textures/lq_tech/tlight11_fbr.png", "start": 192894724, "end": 192896350}, {"filename": "/GameData/textures/lq_tech/tlight12_fbr.png", "start": 192896350, "end": 192898045}, {"filename": "/GameData/textures/lq_tech/tlight13_fbr.png", "start": 192898045, "end": 192900104}, {"filename": "/GameData/textures/lq_tech/tlightblfb_fbr.png", "start": 192900104, "end": 192900677}, {"filename": "/GameData/textures/lq_tech/tlightfb_fbr.png", "start": 192900677, "end": 192901267}, {"filename": "/GameData/textures/lq_tech/tlightnb.png", "start": 192901267, "end": 192901865}, {"filename": "/GameData/textures/lq_tech/tlightrdfb_fbr.png", "start": 192901865, "end": 192902467}, {"filename": "/GameData/textures/lq_tech/treadplatemetal.png", "start": 192902467, "end": 192917221}, {"filename": "/GameData/textures/lq_tech/twall2_3.png", "start": 192917221, "end": 192920729}, {"filename": "/GameData/textures/lq_tech/w17_1.png", "start": 192920729, "end": 192937753}, {"filename": "/GameData/textures/lq_tech/w94_1.png", "start": 192937753, "end": 192950701}, {"filename": "/GameData/textures/lq_tech/z_exit_fbr.png", "start": 192950701, "end": 192952262}, {"filename": "/GameData/textures/lq_terra/afloor1_3.png", "start": 192952262, "end": 192955260}, {"filename": "/GameData/textures/lq_terra/asphalt.png", "start": 192955260, "end": 192972720}, {"filename": "/GameData/textures/lq_terra/azfloor1_1.png", "start": 192972720, "end": 192975739}, {"filename": "/GameData/textures/lq_terra/badlawn.png", "start": 192975739, "end": 193017548}, {"filename": "/GameData/textures/lq_terra/cracks1-1.png", "start": 193017548, "end": 193020567}, {"filename": "/GameData/textures/lq_terra/darkrock.png", "start": 193020567, "end": 193043136}, {"filename": "/GameData/textures/lq_terra/grass1.png", "start": 193043136, "end": 193053707}, {"filename": "/GameData/textures/lq_terra/gravel1.png", "start": 193053707, "end": 193067061}, {"filename": "/GameData/textures/lq_terra/gravel2.png", "start": 193067061, "end": 193081958}, {"filename": "/GameData/textures/lq_terra/grk_leaf1_1.png", "start": 193081958, "end": 193085951}, {"filename": "/GameData/textures/lq_terra/grk_leaf1_2.png", "start": 193085951, "end": 193089688}, {"filename": "/GameData/textures/lq_terra/marbbrn128.png", "start": 193089688, "end": 193099930}, {"filename": "/GameData/textures/lq_terra/may_drt1_1.png", "start": 193099930, "end": 193102826}, {"filename": "/GameData/textures/lq_terra/may_drt1_2.png", "start": 193102826, "end": 193105680}, {"filename": "/GameData/textures/lq_terra/may_drt2_2.png", "start": 193105680, "end": 193108424}, {"filename": "/GameData/textures/lq_terra/may_rck1_1.png", "start": 193108424, "end": 193122524}, {"filename": "/GameData/textures/lq_terra/may_rck1_2.png", "start": 193122524, "end": 193133202}, {"filename": "/GameData/textures/lq_terra/may_rck1_3.png", "start": 193133202, "end": 193146045}, {"filename": "/GameData/textures/lq_terra/may_slat1_1.png", "start": 193146045, "end": 193148893}, {"filename": "/GameData/textures/lq_terra/med_bigdirt.png", "start": 193148893, "end": 193303476}, {"filename": "/GameData/textures/lq_terra/med_bigdirt2.png", "start": 193303476, "end": 193457627}, {"filename": "/GameData/textures/lq_terra/med_bigdirt3.png", "start": 193457627, "end": 193611578}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_1.png", "start": 193611578, "end": 193625959}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_1a.png", "start": 193625959, "end": 193642933}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_2.png", "start": 193642933, "end": 193657372}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_2a.png", "start": 193657372, "end": 193674187}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_1.png", "start": 193674187, "end": 193688156}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_1a.png", "start": 193688156, "end": 193705467}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_2.png", "start": 193705467, "end": 193720541}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_2a.png", "start": 193720541, "end": 193737645}, {"filename": "/GameData/textures/lq_terra/med_cracks1.png", "start": 193737645, "end": 193751107}, {"filename": "/GameData/textures/lq_terra/med_flat1.png", "start": 193751107, "end": 193764016}, {"filename": "/GameData/textures/lq_terra/med_flat12.png", "start": 193764016, "end": 193775404}, {"filename": "/GameData/textures/lq_terra/med_flat15.png", "start": 193775404, "end": 193788505}, {"filename": "/GameData/textures/lq_terra/med_flat16.png", "start": 193788505, "end": 193801347}, {"filename": "/GameData/textures/lq_terra/med_flat2.png", "start": 193801347, "end": 193812624}, {"filename": "/GameData/textures/lq_terra/med_flat3.png", "start": 193812624, "end": 193825989}, {"filename": "/GameData/textures/lq_terra/med_flat4.png", "start": 193825989, "end": 193835938}, {"filename": "/GameData/textures/lq_terra/med_flat5.png", "start": 193835938, "end": 193847023}, {"filename": "/GameData/textures/lq_terra/med_flat5a.png", "start": 193847023, "end": 193857767}, {"filename": "/GameData/textures/lq_terra/med_flat6.png", "start": 193857767, "end": 193866752}, {"filename": "/GameData/textures/lq_terra/med_flat7.png", "start": 193866752, "end": 193878210}, {"filename": "/GameData/textures/lq_terra/med_plaster2.png", "start": 193878210, "end": 193887862}, {"filename": "/GameData/textures/lq_terra/med_rock1.png", "start": 193887862, "end": 193939119}, {"filename": "/GameData/textures/lq_terra/med_rock10.png", "start": 193939119, "end": 193948414}, {"filename": "/GameData/textures/lq_terra/med_rock10a.png", "start": 193948414, "end": 193957728}, {"filename": "/GameData/textures/lq_terra/med_rock10b.png", "start": 193957728, "end": 193971093}, {"filename": "/GameData/textures/lq_terra/med_rock10c.png", "start": 193971093, "end": 193983428}, {"filename": "/GameData/textures/lq_terra/med_rock2.png", "start": 193983428, "end": 194025305}, {"filename": "/GameData/textures/lq_terra/med_rock3.png", "start": 194025305, "end": 194071249}, {"filename": "/GameData/textures/lq_terra/med_rock3_bump.png", "start": 194071249, "end": 194187343}, {"filename": "/GameData/textures/lq_terra/med_rock4.png", "start": 194187343, "end": 194198699}, {"filename": "/GameData/textures/lq_terra/med_rock5.png", "start": 194198699, "end": 194211231}, {"filename": "/GameData/textures/lq_terra/med_rock9.png", "start": 194211231, "end": 194222255}, {"filename": "/GameData/textures/lq_terra/ret_plaster1.png", "start": 194222255, "end": 194233532}, {"filename": "/GameData/textures/lq_terra/rock1_1.png", "start": 194233532, "end": 194283903}, {"filename": "/GameData/textures/lq_terra/rock1_1b.png", "start": 194283903, "end": 194294918}, {"filename": "/GameData/textures/lq_terra/rock1_2.png", "start": 194294918, "end": 194336961}, {"filename": "/GameData/textures/lq_terra/rocks07.png", "start": 194336961, "end": 194347985}, {"filename": "/GameData/textures/lq_terra/rocks11d.png", "start": 194347985, "end": 194357299}, {"filename": "/GameData/textures/lq_terra/rocks11e.png", "start": 194357299, "end": 194366594}, {"filename": "/GameData/textures/lq_terra/sand.png", "start": 194366594, "end": 194392681}, {"filename": "/GameData/textures/lq_terra/snow1.png", "start": 194392681, "end": 194394544}, {"filename": "/GameData/textures/lq_terra/uwall1_2.png", "start": 194394544, "end": 194422195}, {"filename": "/GameData/textures/lq_terra/vines1.png", "start": 194422195, "end": 194427250}, {"filename": "/GameData/textures/lq_utility/black.png", "start": 194427250, "end": 194427798}, {"filename": "/GameData/textures/lq_utility/clip.png", "start": 194427798, "end": 194428304}, {"filename": "/GameData/textures/lq_utility/hint.png", "start": 194428304, "end": 194429170}, {"filename": "/GameData/textures/lq_utility/hintskip.png", "start": 194429170, "end": 194430063}, {"filename": "/GameData/textures/lq_utility/light_fbr.png", "start": 194430063, "end": 194430979}, {"filename": "/GameData/textures/lq_utility/origin.png", "start": 194430979, "end": 194431462}, {"filename": "/GameData/textures/lq_utility/skip.png", "start": 194431462, "end": 194431950}, {"filename": "/GameData/textures/lq_utility/star_lavaskip.png", "start": 194431950, "end": 194433063}, {"filename": "/GameData/textures/lq_utility/star_slimeskip.png", "start": 194433063, "end": 194434136}, {"filename": "/GameData/textures/lq_utility/star_waterskip.png", "start": 194434136, "end": 194435876}, {"filename": "/GameData/textures/lq_utility/trigger.png", "start": 194435876, "end": 194436373}, {"filename": "/GameData/textures/lq_wood/crate4.png", "start": 194436373, "end": 194439515}, {"filename": "/GameData/textures/lq_wood/crwdh6.png", "start": 194439515, "end": 194444523}, {"filename": "/GameData/textures/lq_wood/crwdl12.png", "start": 194444523, "end": 194446956}, {"filename": "/GameData/textures/lq_wood/crwds6.png", "start": 194446956, "end": 194447982}, {"filename": "/GameData/textures/lq_wood/may_crate3-small.png", "start": 194447982, "end": 194448973}, {"filename": "/GameData/textures/lq_wood/may_crate3.png", "start": 194448973, "end": 194452036}, {"filename": "/GameData/textures/lq_wood/may_wood1_1.png", "start": 194452036, "end": 194454876}, {"filename": "/GameData/textures/lq_wood/may_wood1_2.png", "start": 194454876, "end": 194457725}, {"filename": "/GameData/textures/lq_wood/med_ret_wood1.png", "start": 194457725, "end": 194464694}, {"filename": "/GameData/textures/lq_wood/med_wood1.png", "start": 194464694, "end": 194492062}, {"filename": "/GameData/textures/lq_wood/med_wood2.png", "start": 194492062, "end": 194499575}, {"filename": "/GameData/textures/lq_wood/med_wood2_plk1.png", "start": 194499575, "end": 194511273}, {"filename": "/GameData/textures/lq_wood/med_wood2_plk2.png", "start": 194511273, "end": 194523765}, {"filename": "/GameData/textures/lq_wood/med_wood3.png", "start": 194523765, "end": 194529765}, {"filename": "/GameData/textures/lq_wood/med_wood3_plk1.png", "start": 194529765, "end": 194542980}, {"filename": "/GameData/textures/lq_wood/med_wood4.png", "start": 194542980, "end": 194550104}, {"filename": "/GameData/textures/lq_wood/med_wood5.png", "start": 194550104, "end": 194557902}, {"filename": "/GameData/textures/lq_wood/med_wood6.png", "start": 194557902, "end": 194564682}, {"filename": "/GameData/textures/lq_wood/med_wood7.png", "start": 194564682, "end": 194573721}, {"filename": "/GameData/textures/lq_wood/med_wood8.png", "start": 194573721, "end": 194581685}, {"filename": "/GameData/textures/lq_wood/med_wood_riv1.png", "start": 194581685, "end": 194588310}, {"filename": "/GameData/textures/lq_wood/med_wood_riv1b.png", "start": 194588310, "end": 194594389}, {"filename": "/GameData/textures/lq_wood/med_wood_riv1c.png", "start": 194594389, "end": 194599513}, {"filename": "/GameData/textures/lq_wood/med_wood_riv2.png", "start": 194599513, "end": 194605923}, {"filename": "/GameData/textures/lq_wood/med_wood_riv2b.png", "start": 194605923, "end": 194612343}, {"filename": "/GameData/textures/lq_wood/med_wood_riv2c.png", "start": 194612343, "end": 194617715}, {"filename": "/GameData/textures/lq_wood/plank1.png", "start": 194617715, "end": 194624129}, {"filename": "/GameData/textures/lq_wood/plank1s.png", "start": 194624129, "end": 194626010}, {"filename": "/GameData/textures/lq_wood/plank2.png", "start": 194626010, "end": 194632590}, {"filename": "/GameData/textures/lq_wood/plank2s.png", "start": 194632590, "end": 194634479}, {"filename": "/GameData/textures/lq_wood/plank3.png", "start": 194634479, "end": 194641339}, {"filename": "/GameData/textures/lq_wood/plank3s.png", "start": 194641339, "end": 194643320}, {"filename": "/GameData/textures/lq_wood/plank4.png", "start": 194643320, "end": 194649766}, {"filename": "/GameData/textures/lq_wood/plank4s.png", "start": 194649766, "end": 194651656}, {"filename": "/GameData/textures/lq_wood/plank5.png", "start": 194651656, "end": 194657305}, {"filename": "/GameData/textures/lq_wood/sq_wood_1.png", "start": 194657305, "end": 194673346}, {"filename": "/GameData/textures/lq_wood/sq_wood_2.png", "start": 194673346, "end": 194676234}, {"filename": "/GameData/textures/lq_wood/sq_wood_2a.png", "start": 194676234, "end": 194679603}, {"filename": "/GameData/textures/lq_wood/wood_1.png", "start": 194679603, "end": 194683781}, {"filename": "/GameData/textures/lq_wood/wood_2.png", "start": 194683781, "end": 194687681}, {"filename": "/GameData/textures/lq_wood/woodbark128.png", "start": 194687681, "end": 194697888}, {"filename": "/GameData/textures/lq_wood/woodbark1m28.png", "start": 194697888, "end": 194707337}, {"filename": "/GameData/textures/lq_wood/woodbark64.png", "start": 194707337, "end": 194710274}, {"filename": "/GameData/textures/lq_wood/woodbarkA128.png", "start": 194710274, "end": 194718972}, {"filename": "/GameData/textures/lq_wood/woodbarkm64.png", "start": 194718972, "end": 194721649}, {"filename": "/GameData/textures/lq_wood/woodend.png", "start": 194721649, "end": 194724152}, {"filename": "/GameData/textures/lq_wood/woodring128.png", "start": 194724152, "end": 194732576}, {"filename": "/GameData/textures/lq_wood/woodring64.png", "start": 194732576, "end": 194735037}, {"filename": "/GameData/textures/lq_wood/woodringm128.png", "start": 194735037, "end": 194745336}, {"filename": "/GameData/textures/lq_wood/woodringm64.png", "start": 194745336, "end": 194748595}, {"filename": "/GameData/textures/metal/metal1.png", "start": 194748595, "end": 195172651}, {"filename": "/GameData/textures/muzzle_t.png", "start": 195172651, "end": 195182059}, {"filename": "/GameData/textures/muzzle_t_em.png", "start": 195182059, "end": 195191467}, {"filename": "/GameData/textures/noise/grainy5_256.png", "start": 195191467, "end": 195417509}, {"filename": "/GameData/textures/particles/blood.png", "start": 195417509, "end": 195421222}, {"filename": "/GameData/textures/particles/smoke.png", "start": 195421222, "end": 195425092}, {"filename": "/GameData/textures/particles/trail.png", "start": 195425092, "end": 195443962}, {"filename": "/GameData/textures/particles/wood.png", "start": 195443962, "end": 195459445}, {"filename": "/GameData/textures/pp/lut.png", "start": 195459445, "end": 195461068}, {"filename": "/GameData/textures/pp/main.png", "start": 195461068, "end": 195499120}, {"filename": "/GameData/textures/pp/pal.png_out.png", "start": 195499120, "end": 195505470}, {"filename": "/GameData/textures/pp/sin.png", "start": 195505470, "end": 195506795}, {"filename": "/GameData/textures/shirt.png", "start": 195506795, "end": 195698546}, {"filename": "/GameData/textures/skies/skybox1_cube.png", "start": 195698546, "end": 196911513}, {"filename": "/GameData/textures/skies/skybox2_overcast_cube.png", "start": 196911513, "end": 197559352}, {"filename": "/GameData/textures/tormentPack/+0str_bloodfall.png", "start": 197559352, "end": 197561429}, {"filename": "/GameData/textures/tormentPack/+1str_bloodfall.png", "start": 197561429, "end": 197563463}, {"filename": "/GameData/textures/tormentPack/+2str_bloodfall.png", "start": 197563463, "end": 197565561}, {"filename": "/GameData/textures/tormentPack/+3str_bloodfall.png", "start": 197565561, "end": 197567632}, {"filename": "/GameData/textures/tormentPack/+4str_bloodfall.png", "start": 197567632, "end": 197569692}, {"filename": "/GameData/textures/tormentPack/+5str_bloodfall.png", "start": 197569692, "end": 197571713}, {"filename": "/GameData/textures/tormentPack/+6str_bloodfall.png", "start": 197571713, "end": 197573759}, {"filename": "/GameData/textures/tormentPack/+7str_bloodfall.png", "start": 197573759, "end": 197575828}, {"filename": "/GameData/textures/tormentPack/str_blood.png", "start": 197575828, "end": 197577836}, {"filename": "/GameData/textures/tormentPack/str_blood_large.png", "start": 197577836, "end": 197600386}, {"filename": "/GameData/textures/tormentPack/str_bloodvein1.png", "start": 197600386, "end": 197627061}, {"filename": "/GameData/textures/tormentPack/str_bloodvein2.png", "start": 197627061, "end": 197659094}, {"filename": "/GameData/textures/tormentPack/str_bloodvein3.png", "start": 197659094, "end": 197692765}, {"filename": "/GameData/textures/tormentPack/str_bloodvein4.png", "start": 197692765, "end": 197724407}, {"filename": "/GameData/textures/tormentPack/str_bloodvein5.png", "start": 197724407, "end": 197757898}, {"filename": "/GameData/textures/tormentPack/str_bloodvein6.png", "start": 197757898, "end": 197792372}, {"filename": "/GameData/textures/tormentPack/str_bloodvein7.png", "start": 197792372, "end": 197827263}, {"filename": "/GameData/textures/tormentPack/str_bloodvein8.png", "start": 197827263, "end": 197855412}, {"filename": "/GameData/textures/tormentPack/str_bloodvein9.png", "start": 197855412, "end": 197890865}, {"filename": "/GameData/textures/tormentPack/str_metalflr1.png", "start": 197890865, "end": 197927559}, {"filename": "/GameData/textures/tormentPack/str_metalflr2.png", "start": 197927559, "end": 197964134}, {"filename": "/GameData/textures/tormentPack/str_metalflr3.png", "start": 197964134, "end": 197995127}, {"filename": "/GameData/textures/tormentPack/str_metalflr4.png", "start": 197995127, "end": 198025297}, {"filename": "/GameData/textures/tormentPack/str_metalflr5.png", "start": 198025297, "end": 198064747}, {"filename": "/GameData/textures/tormentPack/str_metalflr6.png", "start": 198064747, "end": 198103907}, {"filename": "/GameData/textures/tormentPack/str_metalflr7.png", "start": 198103907, "end": 198145599}, {"filename": "/GameData/textures/tormentPack/str_metalflr8.png", "start": 198145599, "end": 198186017}, {"filename": "/GameData/textures/tormentPack/str_metalgen1.png", "start": 198186017, "end": 198215287}, {"filename": "/GameData/textures/tormentPack/str_metalgen2.png", "start": 198215287, "end": 198244803}, {"filename": "/GameData/textures/tormentPack/str_metalgen3.png", "start": 198244803, "end": 198274279}, {"filename": "/GameData/textures/tormentPack/str_metalgen4.png", "start": 198274279, "end": 198299102}, {"filename": "/GameData/textures/tormentPack/str_metalgen5.png", "start": 198299102, "end": 198323972}, {"filename": "/GameData/textures/tormentPack/str_metalgen6.png", "start": 198323972, "end": 198348062}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl1.png", "start": 198348062, "end": 198377447}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl2.png", "start": 198377447, "end": 198410177}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl3.png", "start": 198410177, "end": 198436795}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl4.png", "start": 198436795, "end": 198466059}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl5.png", "start": 198466059, "end": 198500434}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl6.png", "start": 198500434, "end": 198538929}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl7.png", "start": 198538929, "end": 198575079}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl8.png", "start": 198575079, "end": 198613072}, {"filename": "/GameData/textures/tormentPack/str_metalpan1.png", "start": 198613072, "end": 198647932}, {"filename": "/GameData/textures/tormentPack/str_metalpan2.png", "start": 198647932, "end": 198683473}, {"filename": "/GameData/textures/tormentPack/str_metalpan3.png", "start": 198683473, "end": 198713909}, {"filename": "/GameData/textures/tormentPack/str_metalpan4.png", "start": 198713909, "end": 198744869}, {"filename": "/GameData/textures/tormentPack/str_metalpan5.png", "start": 198744869, "end": 198781241}, {"filename": "/GameData/textures/tormentPack/str_metalpan6.png", "start": 198781241, "end": 198819283}, {"filename": "/GameData/textures/tormentPack/str_metalpan7.png", "start": 198819283, "end": 198858512}, {"filename": "/GameData/textures/tormentPack/str_metalpan8.png", "start": 198858512, "end": 198897606}, {"filename": "/GameData/textures/tormentPack/str_rotwoodgen1.png", "start": 198897606, "end": 198916185}, {"filename": "/GameData/textures/tormentPack/str_rotwoodgen2.png", "start": 198916185, "end": 198935481}, {"filename": "/GameData/textures/tormentPack/str_rotwoodgen3.png", "start": 198935481, "end": 198955535}, {"filename": "/GameData/textures/tormentPack/str_stonebrk1.png", "start": 198955535, "end": 198980927}, {"filename": "/GameData/textures/tormentPack/str_stonebrk2.png", "start": 198980927, "end": 199007929}, {"filename": "/GameData/textures/tormentPack/str_stonebrk3.png", "start": 199007929, "end": 199030559}, {"filename": "/GameData/textures/tormentPack/str_stonebrk4.png", "start": 199030559, "end": 199054612}, {"filename": "/GameData/textures/tormentPack/str_stonebrk5.png", "start": 199054612, "end": 199085696}, {"filename": "/GameData/textures/tormentPack/str_stonebrk6.png", "start": 199085696, "end": 199118368}, {"filename": "/GameData/textures/tormentPack/str_stonebrk7.png", "start": 199118368, "end": 199146892}, {"filename": "/GameData/textures/tormentPack/str_stonebrk8.png", "start": 199146892, "end": 199177144}, {"filename": "/GameData/textures/tormentPack/str_stoneflr1.png", "start": 199177144, "end": 199201300}, {"filename": "/GameData/textures/tormentPack/str_stoneflr2.png", "start": 199201300, "end": 199229206}, {"filename": "/GameData/textures/tormentPack/str_stoneflr3.png", "start": 199229206, "end": 199250211}, {"filename": "/GameData/textures/tormentPack/str_stoneflr4.png", "start": 199250211, "end": 199271355}, {"filename": "/GameData/textures/tormentPack/str_stoneflr5.png", "start": 199271355, "end": 199302887}, {"filename": "/GameData/textures/tormentPack/str_stoneflr6.png", "start": 199302887, "end": 199337170}, {"filename": "/GameData/textures/tormentPack/str_stonegen1.png", "start": 199337170, "end": 199356193}, {"filename": "/GameData/textures/tormentPack/str_stonegen2.png", "start": 199356193, "end": 199375564}, {"filename": "/GameData/textures/tormentPack/str_stonegen3.png", "start": 199375564, "end": 199397336}, {"filename": "/GameData/textures/tormentPack/str_stonegen4.png", "start": 199397336, "end": 199422381}, {"filename": "/GameData/textures/tormentPack/str_stonegen5.png", "start": 199422381, "end": 199447769}, {"filename": "/GameData/textures/tormentPack/str_stonegen6.png", "start": 199447769, "end": 199475206}, {"filename": "/GameData/textures/tormentPack/str_stonerubble.png", "start": 199475206, "end": 199503600}, {"filename": "/GameData/textures/tormentPack/str_stonewall1.png", "start": 199503600, "end": 199526857}, {"filename": "/GameData/textures/tormentPack/str_stonewall2.png", "start": 199526857, "end": 199551662}, {"filename": "/GameData/textures/tormentPack/str_stonewall3.png", "start": 199551662, "end": 199576325}, {"filename": "/GameData/textures/tormentPack/str_stonewall4.png", "start": 199576325, "end": 199602795}, {"filename": "/GameData/textures/tormentPack/str_stonewall5.png", "start": 199602795, "end": 199630262}, {"filename": "/GameData/textures/tormentPack/str_stonewall6.png", "start": 199630262, "end": 199658430}, {"filename": "/GameData/textures/tormentPack/str_stonewall7.png", "start": 199658430, "end": 199687819}, {"filename": "/GameData/textures/tormentPack/str_stonewall8.png", "start": 199687819, "end": 199718715}, {"filename": "/GameData/textures/tormentPack/{str_bloodgunk.png", "start": 199718715, "end": 199745164}, {"filename": "/GameData/textures/tormentPack/{str_bloodweb1.png", "start": 199745164, "end": 199762404}, {"filename": "/GameData/textures/tormentPack/{str_bloodweb2.png", "start": 199762404, "end": 199771355}, {"filename": "/GameData/textures/tormentPack/{str_bloodweb3.png", "start": 199771355, "end": 199777366}, {"filename": "/GameData/textures/tormentPack/{str_grating1.png", "start": 199777366, "end": 199785947}, {"filename": "/GameData/textures/tormentPack/{str_grating2.png", "start": 199785947, "end": 199801150}, {"filename": "/GameData/textures/tormentPack/{str_grating3.png", "start": 199801150, "end": 199820498}, {"filename": "/GameData/textures/tormentPack/{str_grating4.png", "start": 199820498, "end": 199832655}, {"filename": "/GameData/textures/tormentPack/{str_grating5.png", "start": 199832655, "end": 199854160}, {"filename": "/GameData/textures/tormentPack/{str_grating6.png", "start": 199854160, "end": 199882406}, {"filename": "/GameData/textures/ui/circle.png", "start": 199882406, "end": 199885861}, {"filename": "/GameData/textures/ui/crosshair.png", "start": 199885861, "end": 199889976}, {"filename": "/GameData/textures/ui/crosshair_dot.png", "start": 199889976, "end": 199891650}, {"filename": "/GameData/textures/ui/crosshair_line.png", "start": 199891650, "end": 199893589}, {"filename": "/GameData/textures/ui/debuffs/disb.png", "start": 199893589, "end": 200359591}, {"filename": "/GameData/textures/ui/debuffs/qs.png", "start": 200359591, "end": 200846911}, {"filename": "/GameData/textures/ui/debuffs/stun.png", "start": 200846911, "end": 201340788}, {"filename": "/GameData/textures/ui/test_button.png", "start": 201340788, "end": 201344184}, {"filename": "/GameData/textures/ui/white.png", "start": 201344184, "end": 201344305}, {"filename": "/GameData/textures/wall/brickWall1.png", "start": 201344305, "end": 201352521}, {"filename": "/GameData/textures/wall/brickWall2.png", "start": 201352521, "end": 201360645}, {"filename": "/GameData/textures/wall/brickWall3.png", "start": 201360645, "end": 201368528}, {"filename": "/GameData/textures/water/Water1_t.png", "start": 201368528, "end": 201681629}, {"filename": "/GameData/textures/wood/wood1.png", "start": 201681629, "end": 202029981}, {"filename": "/GameData/ui/base.rcss", "start": 202029981, "end": 202031024}, {"filename": "/GameData/ui/pause.rml", "start": 202031024, "end": 202032039}, {"filename": "/GameData/ui/settings.rml", "start": 202032039, "end": 202032907}, {"filename": "/GameData/ui/style.rcss", "start": 202032907, "end": 202037906}, {"filename": "/GameData/ui/test.rml", "start": 202037906, "end": 202039032}, {"filename": "/GameData/ui/videoSettings.rml", "start": 202039032, "end": 202041517}, {"filename": "/GameData/videos/meowl.mpg", "start": 202041517, "end": 205981869}], "remote_package_size": 205981869});

  })();

// end include: C:\Users\bogda_\AppData\Local\Temp\tmpsqib21fx.js
// include: C:\Users\bogda_\AppData\Local\Temp\tmp5_uzdplf.js

    // All the pre-js content up to here must remain later on, we need to run
    // it.
    if ((typeof ENVIRONMENT_IS_WASM_WORKER != 'undefined' && ENVIRONMENT_IS_WASM_WORKER) || (typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD) || (typeof ENVIRONMENT_IS_AUDIO_WORKLET != 'undefined' && ENVIRONMENT_IS_AUDIO_WORKLET)) Module['preRun'] = [];
    var necessaryPreJSTasks = Module['preRun'].slice();
  // end include: C:\Users\bogda_\AppData\Local\Temp\tmp5_uzdplf.js
// include: C:\Users\bogda_\AppData\Local\Temp\tmpcl5292pe.js

    if (!Module['preRun']) throw 'Module.preRun should exist because file support used it; did a pre-js delete it?';
    necessaryPreJSTasks.forEach((task) => {
      if (Module['preRun'].indexOf(task) < 0) throw 'All preRun tasks that exist before user pre-js code should remain after; did you replace Module or modify Module.preRun?';
    });
  // end include: C:\Users\bogda_\AppData\Local\Temp\tmpcl5292pe.js


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

  if (what.indexOf('RuntimeError: unreachable') >= 0) {
    what += '. "unreachable" may be due to ASYNCIFY_STACK_SIZE not being large enough (try increasing it)';
  }

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
  // instrumenting imports is used in asyncify in two ways: to add assertions
  // that check for proper import use, and for ASYNCIFY=2 we use them to set up
  // the Promise API on the import side.
  Asyncify.instrumentWasmImports(wasmImports);
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

    wasmExports = Asyncify.instrumentWasmExports(wasmExports);

    

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


  var dynCallLegacy = (sig, ptr, args) => {
      sig = sig.replace(/p/g, 'i')
      assert(('dynCall_' + sig) in Module, `bad function pointer type - dynCall function not found for sig '${sig}'`);
      if (args?.length) {
        // j (64-bit integer) is fine, and is implemented as a BigInt. Without
        // legalization, the number of parameters should match (j is not expanded
        // into two i's).
        assert(args.length === sig.length - 1);
      } else {
        assert(sig.length == 1);
      }
      var f = Module['dynCall_' + sig];
      return f(ptr, ...args);
    };
  var dynCall = (sig, ptr, args = [], promising = false) => {
      assert(!promising, 'async dynCall is not supported in this mode')
      var rtn = dynCallLegacy(sig, ptr, args);
  
      function convert(rtn) {
        return rtn;
      }
  
      return convert(rtn);
    };

  
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
  asyncLoad.isAsync = true;
  
  
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
            ((a1, a2, a3) => dynCall_iiii(currentFullscreenStrategy.canvasResizedCallback, a1, a2, a3))(37, 0, currentFullscreenStrategy.canvasResizedCallbackUserData);
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
        ((a1, a2, a3) => dynCall_iiii(strategy.canvasResizedCallback, a1, a2, a3))(37, 0, strategy.canvasResizedCallbackUserData);
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

  var _emscripten_has_asyncify = () => 1;

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
        var confirmationMessage = ((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3))(eventTypeId, 0, userData);
  
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
  
        if (((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3))(eventTypeId, focusEvent, userData)) e.preventDefault();
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
  
        if (((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3))(eventTypeId, fullscreenChangeEvent, userData)) e.preventDefault();
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
  
        if (((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3))(eventTypeId, gamepadEvent, userData)) e.preventDefault();
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
  
        if (((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3))(eventTypeId, keyEventData, userData)) e.preventDefault();
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
      var iterFunc = (() => dynCall_v(func));
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
  
        if (((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3))(eventTypeId, JSEvents.mouseEvent, userData)) e.preventDefault();
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
  
        if (((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3))(eventTypeId, pointerlockChangeEvent, userData)) e.preventDefault();
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
        if (((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3))(eventTypeId, uiEvent, userData)) e.preventDefault();
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
  
        if (((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3))(eventTypeId, touchEvent, userData)) e.preventDefault();
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
  
        if (((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3))(eventTypeId, visibilityChangeEvent, userData)) e.preventDefault();
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
        if (((a1, a2, a3) => dynCall_iiii(callbackfunc, a1, a2, a3))(eventTypeId, wheelEvent, userData)) e.preventDefault();
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

  var _emscripten_sleep = (ms) => Asyncify.handleSleep((wakeUp) => safeSetTimeout(wakeUp, ms));
  _emscripten_sleep.isAsync = true;

  
  
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
        if (onsuccess) ((a1) => dynCall_vi(onsuccess, a1))(fetch);
        else successcb?.(fetch);
      });
    };
  
    var reportProgress = (fetch, xhr, e) => {
      doCallback(() => {
        if (onprogress) ((a1) => dynCall_vi(onprogress, a1))(fetch);
        else progresscb?.(fetch);
      });
    };
  
    var reportError = (fetch, xhr, e) => {
      
      doCallback(() => {
        if (onerror) ((a1) => dynCall_vi(onerror, a1))(fetch);
        else errorcb?.(fetch);
      });
    };
  
    var reportReadyStateChange = (fetch, xhr, e) => {
      doCallback(() => {
        if (onreadystatechange) ((a1) => dynCall_vi(onreadystatechange, a1))(fetch);
        else readystatechangecb?.(fetch);
      });
    };
  
    var performUncachedXhr = (fetch, xhr, e) => {
      fetchXHR(fetch, reportSuccess, reportError, reportProgress, reportReadyStateChange);
    };
  
    var cacheResultAndReportSuccess = (fetch, xhr, e) => {
      var storeSuccess = (fetch, xhr, e) => {
        
        doCallback(() => {
          if (onsuccess) ((a1) => dynCall_vi(onsuccess, a1))(fetch);
          else successcb?.(fetch);
        });
      };
      var storeError = (fetch, xhr, e) => {
        
        doCallback(() => {
          if (onsuccess) ((a1) => dynCall_vi(onsuccess, a1))(fetch);
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


  var runAndAbortIfError = (func) => {
      try {
        return func();
      } catch (e) {
        abort(e);
      }
    };
  
  
  var sigToWasmTypes = (sig) => {
      var typeNames = {
        'i': 'i32',
        'j': 'i64',
        'f': 'f32',
        'd': 'f64',
        'e': 'externref',
        'p': 'i32',
      };
      var type = {
        parameters: [],
        results: sig[0] == 'v' ? [] : [typeNames[sig[0]]]
      };
      for (var i = 1; i < sig.length; ++i) {
        assert(sig[i] in typeNames, 'invalid signature char: ' + sig[i]);
        type.parameters.push(typeNames[sig[i]]);
      }
      return type;
    };
  
  var runtimeKeepalivePush = () => {
      runtimeKeepaliveCounter += 1;
    };
  
  var runtimeKeepalivePop = () => {
      assert(runtimeKeepaliveCounter > 0);
      runtimeKeepaliveCounter -= 1;
    };
  
  
  var Asyncify = {
  instrumentWasmImports(imports) {
        var importPattern = /^(invoke_.*|__asyncjs__.*)$/;
  
        for (let [x, original] of Object.entries(imports)) {
          if (typeof original == 'function') {
            let isAsyncifyImport = original.isAsync || importPattern.test(x);
            imports[x] = (...args) => {
              var originalAsyncifyState = Asyncify.state;
              try {
                return original(...args);
              } finally {
                // Only asyncify-declared imports are allowed to change the
                // state.
                // Changing the state from normal to disabled is allowed (in any
                // function) as that is what shutdown does (and we don't have an
                // explicit list of shutdown imports).
                var changedToDisabled =
                      originalAsyncifyState === Asyncify.State.Normal &&
                      Asyncify.state        === Asyncify.State.Disabled;
                // invoke_* functions are allowed to change the state if we do
                // not ignore indirect calls.
                var ignoredInvoke = x.startsWith('invoke_') &&
                                    true;
                if (Asyncify.state !== originalAsyncifyState &&
                    !isAsyncifyImport &&
                    !changedToDisabled &&
                    !ignoredInvoke) {
                  throw new Error(`import ${x} was not in ASYNCIFY_IMPORTS, but changed the state`);
                }
              }
            };
          }
        }
      },
  instrumentWasmExports(exports) {
        var ret = {};
        for (let [x, original] of Object.entries(exports)) {
          if (typeof original == 'function') {
            ret[x] = (...args) => {
              Asyncify.exportCallStack.push(x);
              try {
                return original(...args);
              } finally {
                if (!ABORT) {
                  var y = Asyncify.exportCallStack.pop();
                  assert(y === x);
                  Asyncify.maybeStopUnwind();
                }
              }
            };
          } else {
            ret[x] = original;
          }
        }
        return ret;
      },
  State:{
  Normal:0,
  Unwinding:1,
  Rewinding:2,
  Disabled:3,
  },
  state:0,
  StackSize:4096,
  currData:null,
  handleSleepReturnValue:0,
  exportCallStack:[],
  callStackNameToId:{
  },
  callStackIdToName:{
  },
  callStackId:0,
  asyncPromiseHandlers:null,
  sleepCallbacks:[],
  getCallStackId(funcName) {
        var id = Asyncify.callStackNameToId[funcName];
        if (id === undefined) {
          id = Asyncify.callStackId++;
          Asyncify.callStackNameToId[funcName] = id;
          Asyncify.callStackIdToName[id] = funcName;
        }
        return id;
      },
  maybeStopUnwind() {
        if (Asyncify.currData &&
            Asyncify.state === Asyncify.State.Unwinding &&
            Asyncify.exportCallStack.length === 0) {
          // We just finished unwinding.
          // Be sure to set the state before calling any other functions to avoid
          // possible infinite recursion here (For example in debug pthread builds
          // the dbg() function itself can call back into WebAssembly to get the
          // current pthread_self() pointer).
          Asyncify.state = Asyncify.State.Normal;
          
          // Keep the runtime alive so that a re-wind can be done later.
          runAndAbortIfError(_asyncify_stop_unwind);
          if (typeof Fibers != 'undefined') {
            Fibers.trampoline();
          }
        }
      },
  whenDone() {
        assert(Asyncify.currData, 'Tried to wait for an async operation when none is in progress.');
        assert(!Asyncify.asyncPromiseHandlers, 'Cannot have multiple async operations in flight at once');
        return new Promise((resolve, reject) => {
          Asyncify.asyncPromiseHandlers = { resolve, reject };
        });
      },
  allocateData() {
        // An asyncify data structure has three fields:
        //  0  current stack pos
        //  4  max stack pos
        //  8  id of function at bottom of the call stack (callStackIdToName[id] == name of js function)
        //
        // The Asyncify ABI only interprets the first two fields, the rest is for the runtime.
        // We also embed a stack in the same memory region here, right next to the structure.
        // This struct is also defined as asyncify_data_t in emscripten/fiber.h
        var ptr = _malloc(12 + Asyncify.StackSize);
        Asyncify.setDataHeader(ptr, ptr + 12, Asyncify.StackSize);
        Asyncify.setDataRewindFunc(ptr);
        return ptr;
      },
  setDataHeader(ptr, stack, stackSize) {
        HEAPU32[((ptr)>>2)] = stack;
        HEAPU32[(((ptr)+(4))>>2)] = stack + stackSize;
      },
  setDataRewindFunc(ptr) {
        var bottomOfCallStack = Asyncify.exportCallStack[0];
        var rewindId = Asyncify.getCallStackId(bottomOfCallStack);
        HEAP32[(((ptr)+(8))>>2)] = rewindId;
      },
  getDataRewindFuncName(ptr) {
        var id = HEAP32[(((ptr)+(8))>>2)];
        var name = Asyncify.callStackIdToName[id];
        return name;
      },
  getDataRewindFunc(name) {
        var func = wasmExports[name];
        return func;
      },
  doRewind(ptr) {
        var name = Asyncify.getDataRewindFuncName(ptr);
        var func = Asyncify.getDataRewindFunc(name);
        // Once we have rewound and the stack we no longer need to artificially
        // keep the runtime alive.
        
        return func();
      },
  handleSleep(startAsync) {
        assert(Asyncify.state !== Asyncify.State.Disabled, 'Asyncify cannot be done during or after the runtime exits');
        if (ABORT) return;
        if (Asyncify.state === Asyncify.State.Normal) {
          // Prepare to sleep. Call startAsync, and see what happens:
          // if the code decided to call our callback synchronously,
          // then no async operation was in fact begun, and we don't
          // need to do anything.
          var reachedCallback = false;
          var reachedAfterCallback = false;
          startAsync((handleSleepReturnValue = 0) => {
            assert(!handleSleepReturnValue || typeof handleSleepReturnValue == 'number' || typeof handleSleepReturnValue == 'boolean'); // old emterpretify API supported other stuff
            if (ABORT) return;
            Asyncify.handleSleepReturnValue = handleSleepReturnValue;
            reachedCallback = true;
            if (!reachedAfterCallback) {
              // We are happening synchronously, so no need for async.
              return;
            }
            // This async operation did not happen synchronously, so we did
            // unwind. In that case there can be no compiled code on the stack,
            // as it might break later operations (we can rewind ok now, but if
            // we unwind again, we would unwind through the extra compiled code
            // too).
            assert(!Asyncify.exportCallStack.length, 'Waking up (starting to rewind) must be done from JS, without compiled code on the stack.');
            Asyncify.state = Asyncify.State.Rewinding;
            runAndAbortIfError(() => _asyncify_start_rewind(Asyncify.currData));
            if (typeof MainLoop != 'undefined' && MainLoop.func) {
              MainLoop.resume();
            }
            var asyncWasmReturnValue, isError = false;
            try {
              asyncWasmReturnValue = Asyncify.doRewind(Asyncify.currData);
            } catch (err) {
              asyncWasmReturnValue = err;
              isError = true;
            }
            // Track whether the return value was handled by any promise handlers.
            var handled = false;
            if (!Asyncify.currData) {
              // All asynchronous execution has finished.
              // `asyncWasmReturnValue` now contains the final
              // return value of the exported async WASM function.
              //
              // Note: `asyncWasmReturnValue` is distinct from
              // `Asyncify.handleSleepReturnValue`.
              // `Asyncify.handleSleepReturnValue` contains the return
              // value of the last C function to have executed
              // `Asyncify.handleSleep()`, where as `asyncWasmReturnValue`
              // contains the return value of the exported WASM function
              // that may have called C functions that
              // call `Asyncify.handleSleep()`.
              var asyncPromiseHandlers = Asyncify.asyncPromiseHandlers;
              if (asyncPromiseHandlers) {
                Asyncify.asyncPromiseHandlers = null;
                (isError ? asyncPromiseHandlers.reject : asyncPromiseHandlers.resolve)(asyncWasmReturnValue);
                handled = true;
              }
            }
            if (isError && !handled) {
              // If there was an error and it was not handled by now, we have no choice but to
              // rethrow that error into the global scope where it can be caught only by
              // `onerror` or `onunhandledpromiserejection`.
              throw asyncWasmReturnValue;
            }
          });
          reachedAfterCallback = true;
          if (!reachedCallback) {
            // A true async operation was begun; start a sleep.
            Asyncify.state = Asyncify.State.Unwinding;
            // TODO: reuse, don't alloc/free every sleep
            Asyncify.currData = Asyncify.allocateData();
            if (typeof MainLoop != 'undefined' && MainLoop.func) {
              MainLoop.pause();
            }
            runAndAbortIfError(() => _asyncify_start_unwind(Asyncify.currData));
          }
        } else if (Asyncify.state === Asyncify.State.Rewinding) {
          // Stop a resume.
          Asyncify.state = Asyncify.State.Normal;
          runAndAbortIfError(_asyncify_stop_rewind);
          _free(Asyncify.currData);
          Asyncify.currData = null;
          // Call all sleep callbacks now that the sleep-resume is all done.
          Asyncify.sleepCallbacks.forEach(callUserCallback);
        } else {
          abort(`invalid state: ${Asyncify.state}`);
        }
        return Asyncify.handleSleepReturnValue;
      },
  handleAsync:(startAsync) => Asyncify.handleSleep((wakeUp) => {
        // TODO: add error handling as a second param when handleSleep implements it.
        startAsync().then(wakeUp);
      }),
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
      // Data for a previous async operation that was in flight before us.
      var previousAsync = Asyncify.currData;
      var ret = func(...cArgs);
      function onDone(ret) {
        runtimeKeepalivePop();
        if (stack !== 0) stackRestore(stack);
        return convertReturnValue(ret);
      }
    var asyncMode = opts?.async;
  
      // Keep the runtime alive through all calls. Note that this call might not be
      // async, but for simplicity we push and pop in all calls.
      runtimeKeepalivePush();
      if (Asyncify.currData != previousAsync) {
        // A change in async operation happened. If there was already an async
        // operation in flight before us, that is an error: we should not start
        // another async operation while one is active, and we should not stop one
        // either. The only valid combination is to have no change in the async
        // data (so we either had one in flight and left it alone, or we didn't have
        // one), or to have nothing in flight and to start one.
        assert(!(previousAsync && Asyncify.currData), 'We cannot start an async operation when one is already flight');
        assert(!(previousAsync && !Asyncify.currData), 'We cannot stop an async operation in flight');
        // This is a new async operation. The wasm is paused and has unwound its stack.
        // We need to return a Promise that resolves the return value
        // once the stack is rewound and execution finishes.
        assert(asyncMode, 'The call to ' + ident + ' is running asynchronously. If this was intended, add the async option to the ccall/cwrap call.');
        return Asyncify.whenDone().then(onDone);
      }
  
      ret = onDone(ret);
      // If this is an async ccall, ensure we return a promise
      if (asyncMode) return Promise.resolve(ret);
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
  'dynCallLegacy',
  'dynCall',
  'handleException',
  'keepRuntimeAlive',
  'runtimeKeepalivePush',
  'runtimeKeepalivePop',
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
  'sigToWasmTypes',
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
  'runAndAbortIfError',
  'Asyncify',
  'Fibers',
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
  1385579: ($0) => { var str = UTF8ToString($0) + '\n\n' + 'Abort/Retry/Ignore/AlwaysIgnore? [ariA] :'; var reply = window.prompt(str, "i"); if (reply === null) { reply = "i"; } return reply.length === 1 ? reply.charCodeAt(0) : -1; },  
 1385794: () => { if (typeof(AudioContext) !== 'undefined') { return true; } else if (typeof(webkitAudioContext) !== 'undefined') { return true; } return false; },  
 1385941: () => { if ((typeof(navigator.mediaDevices) !== 'undefined') && (typeof(navigator.mediaDevices.getUserMedia) !== 'undefined')) { return true; } else if (typeof(navigator.webkitGetUserMedia) !== 'undefined') { return true; } return false; },  
 1386175: ($0) => { if(typeof(Module['SDL2']) === 'undefined') { Module['SDL2'] = {}; } var SDL2 = Module['SDL2']; if (!$0) { SDL2.audio = {}; } else { SDL2.capture = {}; } if (!SDL2.audioContext) { if (typeof(AudioContext) !== 'undefined') { SDL2.audioContext = new AudioContext(); } else if (typeof(webkitAudioContext) !== 'undefined') { SDL2.audioContext = new webkitAudioContext(); } if (SDL2.audioContext) { if ((typeof navigator.userActivation) === 'undefined') { autoResumeAudioContext(SDL2.audioContext); } } } return SDL2.audioContext === undefined ? -1 : 0; },  
 1386727: () => { var SDL2 = Module['SDL2']; return SDL2.audioContext.sampleRate; },  
 1386795: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; var have_microphone = function(stream) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); SDL2.capture.silenceTimer = undefined; SDL2.capture.silenceBuffer = undefined } SDL2.capture.mediaStreamNode = SDL2.audioContext.createMediaStreamSource(stream); SDL2.capture.scriptProcessorNode = SDL2.audioContext.createScriptProcessor($1, $0, 1); SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) { if ((SDL2 === undefined) || (SDL2.capture === undefined)) { return; } audioProcessingEvent.outputBuffer.getChannelData(0).fill(0.0); SDL2.capture.currentCaptureBuffer = audioProcessingEvent.inputBuffer; dynCall('vp', $2, [$3]); }; SDL2.capture.mediaStreamNode.connect(SDL2.capture.scriptProcessorNode); SDL2.capture.scriptProcessorNode.connect(SDL2.audioContext.destination); SDL2.capture.stream = stream; }; var no_microphone = function(error) { }; SDL2.capture.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.capture.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { SDL2.capture.currentCaptureBuffer = SDL2.capture.silenceBuffer; dynCall('vp', $2, [$3]); }; SDL2.capture.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); if ((navigator.mediaDevices !== undefined) && (navigator.mediaDevices.getUserMedia !== undefined)) { navigator.mediaDevices.getUserMedia({ audio: true, video: false }).then(have_microphone).catch(no_microphone); } else if (navigator.webkitGetUserMedia !== undefined) { navigator.webkitGetUserMedia({ audio: true, video: false }, have_microphone, no_microphone); } },  
 1388488: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; SDL2.audio.scriptProcessorNode = SDL2.audioContext['createScriptProcessor']($1, 0, $0); SDL2.audio.scriptProcessorNode['onaudioprocess'] = function (e) { if ((SDL2 === undefined) || (SDL2.audio === undefined)) { return; } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); SDL2.audio.silenceTimer = undefined; SDL2.audio.silenceBuffer = undefined; } SDL2.audio.currentOutputBuffer = e['outputBuffer']; dynCall('vp', $2, [$3]); }; SDL2.audio.scriptProcessorNode['connect'](SDL2.audioContext['destination']); if (SDL2.audioContext.state === 'suspended') { SDL2.audio.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.audio.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { if ((typeof navigator.userActivation) !== 'undefined') { if (navigator.userActivation.hasBeenActive) { SDL2.audioContext.resume(); } } SDL2.audio.currentOutputBuffer = SDL2.audio.silenceBuffer; dynCall('vp', $2, [$3]); SDL2.audio.currentOutputBuffer = undefined; }; SDL2.audio.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); } },  
 1389663: ($0, $1) => { var SDL2 = Module['SDL2']; var numChannels = SDL2.capture.currentCaptureBuffer.numberOfChannels; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.capture.currentCaptureBuffer.getChannelData(c); if (channelData.length != $1) { throw 'Web Audio capture buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } if (numChannels == 1) { for (var j = 0; j < $1; ++j) { setValue($0 + (j * 4), channelData[j], 'float'); } } else { for (var j = 0; j < $1; ++j) { setValue($0 + (((j * numChannels) + c) * 4), channelData[j], 'float'); } } } },  
 1390268: ($0, $1) => { var SDL2 = Module['SDL2']; var buf = $0 >>> 2; var numChannels = SDL2.audio.currentOutputBuffer['numberOfChannels']; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.audio.currentOutputBuffer['getChannelData'](c); if (channelData.length != $1) { throw 'Web Audio output buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } for (var j = 0; j < $1; ++j) { channelData[j] = HEAPF32[buf + (j*numChannels + c)]; } } },  
 1390757: ($0) => { var SDL2 = Module['SDL2']; if ($0) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); } if (SDL2.capture.stream !== undefined) { var tracks = SDL2.capture.stream.getAudioTracks(); for (var i = 0; i < tracks.length; i++) { SDL2.capture.stream.removeTrack(tracks[i]); } } if (SDL2.capture.scriptProcessorNode !== undefined) { SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) {}; SDL2.capture.scriptProcessorNode.disconnect(); } if (SDL2.capture.mediaStreamNode !== undefined) { SDL2.capture.mediaStreamNode.disconnect(); } SDL2.capture = undefined; } else { if (SDL2.audio.scriptProcessorNode != undefined) { SDL2.audio.scriptProcessorNode.disconnect(); } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); } SDL2.audio = undefined; } if ((SDL2.audioContext !== undefined) && (SDL2.audio === undefined) && (SDL2.capture === undefined)) { SDL2.audioContext.close(); SDL2.audioContext = undefined; } },  
 1391763: ($0, $1, $2) => { var w = $0; var h = $1; var pixels = $2; if (!Module['SDL2']) Module['SDL2'] = {}; var SDL2 = Module['SDL2']; if (SDL2.ctxCanvas !== Module['canvas']) { SDL2.ctx = Browser.createContext(Module['canvas'], false, true); SDL2.ctxCanvas = Module['canvas']; } if (SDL2.w !== w || SDL2.h !== h || SDL2.imageCtx !== SDL2.ctx) { SDL2.image = SDL2.ctx.createImageData(w, h); SDL2.w = w; SDL2.h = h; SDL2.imageCtx = SDL2.ctx; } var data = SDL2.image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = 0xff; src++; dst += 4; } } else { if (SDL2.data32Data !== data) { SDL2.data32 = new Int32Array(data.buffer); SDL2.data8 = new Uint8Array(data.buffer); SDL2.data32Data = data; } var data32 = SDL2.data32; num = data32.length; data32.set(HEAP32.subarray(src, src + num)); var data8 = SDL2.data8; var i = 3; var j = i + 4*num; if (num % 8 == 0) { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; } } else { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; } } } SDL2.ctx.putImageData(SDL2.image, 0, 0); },  
 1393229: ($0, $1, $2, $3, $4) => { var w = $0; var h = $1; var hot_x = $2; var hot_y = $3; var pixels = $4; var canvas = document.createElement("canvas"); canvas.width = w; canvas.height = h; var ctx = canvas.getContext("2d"); var image = ctx.createImageData(w, h); var data = image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = (val >> 24) & 0xff; src++; dst += 4; } } else { var data32 = new Int32Array(data.buffer); num = data32.length; data32.set(HEAP32.subarray(src, src + num)); } ctx.putImageData(image, 0, 0); var url = hot_x === 0 && hot_y === 0 ? "url(" + canvas.toDataURL() + "), auto" : "url(" + canvas.toDataURL() + ") " + hot_x + " " + hot_y + ", auto"; var urlBuf = _malloc(url.length + 1); stringToUTF8(url, urlBuf, url.length + 1); return urlBuf; },  
 1394217: ($0) => { if (Module['canvas']) { Module['canvas'].style['cursor'] = UTF8ToString($0); } },  
 1394300: () => { if (Module['canvas']) { Module['canvas'].style['cursor'] = 'none'; } },  
 1394369: () => { return window.innerWidth; },  
 1394399: () => { return window.innerHeight; },  
 1394430: () => { return Module.window ? 1 : 0; },  
 1394464: () => { var win = Module.window ? Module.window : window; var AudioContext = win.AudioContext || win.webkitAudioContext; var contextForCheck = new AudioContext(); if (!contextForCheck) { return 0; } var retValue = 0; if (self.AudioWorkletNode) { if (contextForCheck.audioWorklet.addModule) { retValue = 1; } } contextForCheck.close(); return retValue; },  
 1394812: () => { var win = Module.window ? Module.window : window; var AudioContext = win.AudioContext || win.webkitAudioContext; var infocontext = new AudioContext(); if (!infocontext) { return 0; } var inforate = infocontext.sampleRate; infocontext.close(); return inforate; },  
 1395076: () => { Module.mInputRegistered = false; },  
 1395113: () => { var win = Module.window ? Module.window : window; var AudioContext = win.AudioContext || win.webkitAudioContext; Module.context = new AudioContext(); if (!Module.context) { return 0; } Module.FMOD_JS_MixFunction = Module["cwrap"]('FMOD_JS_MixFunction', 'void', ['number']); return Module.context.sampleRate; },  
 1395425: ($0, $1) => { Module._as_script_node = Module.context.createScriptProcessor($1, 0, $0); Module["OutputWebAudio_resumeAudio"] = function() { if (Module.context && Module.mInputRegistered) { console.log('Resetting audio driver based on user input.'); Module._as_script_node.connect(Module.context.destination); Module._as_script_node.onaudioprocess = function(audioProcessingEvent) { Module._as_output_buffer = audioProcessingEvent.outputBuffer; Module.FMOD_JS_MixFunction(Module._as_output_buffer.getChannelData(0).length); }; Module.context.resume(); var win = Module.window ? Module.window : window; win.removeEventListener('click', Module.OutputWebAudio_resumeAudio, false); win.removeEventListener('touchend', Module.OutputWebAudio_resumeAudio, false); Module.mInputRegistered = false; } }; },  
 1396209: () => { if (Module.mInputRegistered) { Module.mInputRegistered = false; var win = Module.window ? Module.window : window; win.removeEventListener('click', Module["OutputWebAudio_resumeAudio"], false); win.removeEventListener('touchend', Module["OutputWebAudio_resumeAudio"], false); } },  
 1396490: () => { var win = Module.window ? Module.window : window; win.addEventListener('touchend', Module["OutputWebAudio_resumeAudio"], false); win.addEventListener('click', Module["OutputWebAudio_resumeAudio"], false); Module.mInputRegistered = true; },  
 1396731: () => { Module._as_script_node.disconnect(Module.context.destination); },  
 1396798: ($0, $1, $2, $3) => { var data = HEAPF32.subarray(($0 / 4), ($0 / 4) + ($2 * $3)); for (var channel = 0; channel < $3; channel++) { var outputData = Module._as_output_buffer.getChannelData(channel); for (var sample = 0; sample < $2; sample++) { outputData[sample+$1] = data[(sample*$3)+channel]; } } },  
 1397080: () => { Module.context.suspend(); },  
 1397110: () => { Module.context.resume(); },  
 1397139: () => { var win = Module.window ? Module.window : window; var AudioContext = win.AudioContext || win.webkitAudioContext; var infocontext = new AudioContext(); if (!infocontext) { return 0; } var inforate = infocontext.sampleRate; infocontext.close(); return inforate; },  
 1397403: () => { Module.mWorkletNode = null; Module.mModulePolling = false; Module.mModuleLoading = false; Module.mStartInterval = null; Module.mStopInterval = null; Module.mSuspendInterval = null; Module.mResumeInterval = null; Module.mWorkletNodeConnected = false; Module.mInputRegistered = false; var win = Module.window ? Module.window : window; var AudioContext = win.AudioContext || win.webkitAudioContext; Module.mContext = new AudioContext(); if (!Module.mContext) { return 0; } Module.mContext.destination.channelCount = Module.mContext.destination.maxChannelCount; return Module.mContext.destination.maxChannelCount; },  
 1398017: ($0) => { const initAddModuleRef = $0; if (!initAddModuleRef) { Module.mAddModuleRef = 0; } if (!self.AudioWorkletNode) { return -1; } if (!Module.mContext.audioWorklet.addModule) { return -2; } Module.FMOD_JS_MixerSlowpathFunction = Module["cwrap"]('FMOD_JS_MixerSlowpathFunction', 'void', []); Module.FMOD_JS_MixerFastpathFunction = Module["cwrap"]('FMOD_JS_MixerFastpathFunction', 'void', ['number']); return Module.mContext.sampleRate; },  
 1398451: ($0, $1) => { Module.mSpeakerChannelCount = $0; const bufferLength = $1; Module.mUrl = null; Module.mOutputData = null; Module.mSharedArrayBuffers = false; if (self.SharedArrayBuffer) { if (self.crossOriginIsolated) { Module.mSharedArrayBuffers = true; } } if (!Module.mSharedArrayBuffers) { const slowCodePath = new Blob( [ "class AudioProcessor extends AudioWorkletProcessor", "{", "constructor(options)", "{", "super();", "this.payload = [null, null];", "this.bufferFlag = 0;", "this.dataFlag = 0;", "this.bufferIndex = 0;", "this.bufferSize = 0;", "this.channelCount = options.outputChannelCount;", "this.port.onmessage = (event) => {", "const { data } = event;", "if (data)", "{", "this.payload[this.dataFlag] = new Float32Array(data);", "this.bufferSize = data.length / this.channelCount;", "}", "else", "{", "this.payload[this.dataFlag] = null;", "}", "this.dataFlag ^= 1;", "};", "}", "process(inputs, outputs, parameters)", "{", "const output = outputs[0];", "if (output.length === 0) return true;", "if (this.payload[this.bufferFlag]) {", "if (this.bufferIndex === 0) {", "this.port.postMessage(this.bufferSize);", "}", "const bufferSliceEnd = this.bufferSize / output[0].length;", "const sliceOffset = output[0].length * this.bufferIndex;", "for (let channel = 0; channel < output.length; ++channel) {", "const outputChannel = output[channel];", "const indexOffset = this.bufferSize * channel + sliceOffset;", "outputChannel.set(new Float32Array(this.payload[this.bufferFlag].slice(0 + indexOffset, outputChannel.length + indexOffset)));", "}", "this.bufferIndex++;", "if (this.bufferIndex === bufferSliceEnd) {", "this.bufferIndex = 0;", "this.bufferFlag ^= 1;", "}", "}", "return true;", "}", "}", "registerProcessor('audio-processor', AudioProcessor);" ], { type: 'application/javascript' }); Module.mUrl = URL.createObjectURL(slowCodePath); Module.mOutputData = new Float32Array(Module.mSpeakerChannelCount * bufferLength); } else { const fastCodePath = new Blob( [ "class AudioProcessor extends AudioWorkletProcessor", "{", "constructor(options)", "{", "super();", "this.payload = null;", "this.bufferFlag = 0;", "this.bufferIndex = 0;", "this.bufferSize = 0;", "this.bufferOffset = 0;", "this.channelCount = options.outputChannelCount;", "this.port.onmessage = (event) => {", "const { data } = event;", "if (data)", "{", "this.payload = data;", "this.bufferSize = this.payload.length / (this.channelCount * 2);", "this.bufferOffset = this.payload.length / 2;", "}", "else", "{", "this.payload = null;", "}", "};", "}", "process(inputs, outputs, parameters)", "{", "const output = outputs[0];", "if (output.length === 0) return true;", "if (this.payload) {", "if (this.bufferIndex === 0) {", "this.port.postMessage(this.bufferFlag ^ 1);", "}", "const bufferSliceEnd = this.bufferSize / output[0].length;", "const sliceOffset = output[0].length * this.bufferIndex + this.bufferOffset * this.bufferFlag;", "for (let channel = 0; channel < output.length; ++channel) {", "const outputChannel = output[channel];", "const indexOffset = this.bufferSize * channel + sliceOffset;", "outputChannel.set(new Float32Array(this.payload.slice(0 + indexOffset, outputChannel.length + indexOffset)));", "}", "this.bufferIndex++;", "if (this.bufferIndex === bufferSliceEnd) {", "this.bufferIndex = 0;", "this.bufferFlag ^= 1;", "}", "}", "return true;", "}", "}", "registerProcessor('audio-processor', AudioProcessor);" ], { type: 'application/javascript' }); Module.mUrl = URL.createObjectURL(fastCodePath); Module.mOutputData = new Float32Array(new SharedArrayBuffer(Float32Array.BYTES_PER_ELEMENT * Module.mSpeakerChannelCount * bufferLength * 2)); } Module.mModulePolling = true; Module["waitForAudioWorklet"] = function(condition, callback) { var myInterval = null; if (condition()) { myInterval = setInterval(function() { if (!condition()) { callback(); clearInterval(myInterval); } }, 50); } else { callback(); return null; } return myInterval; }; Module["OutputAudioWorklet_resumeAudio"] = function() { if (Module.mContext && Module.mInputRegistered) { console.log('Resetting audio driver based on user input.'); Module.mContext.resume(); var win = Module.window ? Module.window : window; win.removeEventListener('click', Module.OutputAudioWorklet_resumeAudio, false); win.removeEventListener('touchend', Module.OutputAudioWorklet_resumeAudio, false); Module.mInputRegistered = false; if (!Module.mModuleLoading) { Module.mModuleLoading = true; Module.mAddModuleRef++; Module.mContext.resume().then(function() { Module.mContext.audioWorklet.addModule(Module.mUrl).then(function() { if (Module.mAddModuleRef === 1) { Module.mWorkletNode = new AudioWorkletNode(Module.mContext, 'audio-processor', { 'outputChannelCount' : [Module.mSpeakerChannelCount] }); Module.mModulePolling = false; URL.revokeObjectURL(Module.mUrl); if (Module.mWorkletNode) { Module.mWorkletNode.port.postMessage(Module.mOutputData); if (Module.mSharedArrayBuffers) { Module.mWorkletNode.port.onmessage = function(event) { Module.FMOD_JS_MixerFastpathFunction(event.data); }; } else { Module.mWorkletNode.port.onmessage = function(event) { Module.FMOD_JS_MixerSlowpathFunction(); Module.mWorkletNode.port.postMessage(Module.mOutputData); }; } } else { console.log('Error when creating AudioWorkletNode: Null object'); } } Module.mAddModuleRef--; }).catch (function(err) { Module.mModulePolling = false; Module.mAddModuleRef--; console.log('Error when opening audio processor '); console.log(err) }); }).catch (function(err) { Module.mModulePolling = false; Module.mAddModuleRef--; console.log('Error with mContext.resume()'); console.log(err) }); } } }; return Module.mSharedArrayBuffers; },  
 1404106: () => { var win = Module.window ? Module.window : window; win.addEventListener('touchend', Module["OutputAudioWorklet_resumeAudio"], false); win.addEventListener('click', Module["OutputAudioWorklet_resumeAudio"], false); Module.mInputRegistered = true; },  
 1404355: () => { if (Module.mStartInterval) { clearInterval(Module.mStartInterval); } if (Module.mStopInterval) { clearInterval(Module.mStopInterval); } if (Module.mSuspendInterval) { clearInterval(Module.mSuspendInterval); } if (Module.mResumeInterval) { clearInterval(Module.mResumeInterval); } if (Module.mWorkletNode) { if (Module.mWorkletNodeConnected) { Module.mWorkletNode.disconnect(); } } if (Module.mContext) { Module.mContext.close(); } if (Module.mInputRegistered) { Module.mInputRegistered = false; var win = Module.window ? Module.window : window; win.removeEventListener('click', Module["OutputAudioWorklet_resumeAudio"], false); win.removeEventListener('touchend', Module["OutputAudioWorklet_resumeAudio"], false); } },  
 1405075: () => { Module.mStartInterval = Module["waitForAudioWorklet"](function(){ return (Module.mWorkletNode === null && Module.mModulePolling); }, function(){ if (Module.mWorkletNode) { Module.mWorkletNode.connect(Module.mContext.destination); Module.mWorkletNodeConnected = true; } }); },  
 1405352: () => { if (Module["waitForAudioWorklet"]) { Module.mStopInterval = Module["waitForAudioWorklet"](function(){ return (Module.mWorkletNode === null && Module.mModulePolling || !Module.mWorkletNodeConnected); }, function(){ if (Module.mWorkletNode) { Module.mWorkletNode.disconnect(); Module.mWorkletNodeConnected = false; } }); } },  
 1405677: ($0, $1, $2) => { const buffer = $0; const bufferLength = $1; const speakerModeChannels = $2; var data = HEAPF32.subarray((buffer / 4), (buffer / 4) + (bufferLength * speakerModeChannels)); for (var channel = 0; channel < speakerModeChannels; channel++) { const offset = channel * bufferLength; for (var sample = 0; sample < bufferLength; sample++) { Module.mOutputData[sample + offset] = data[(sample * speakerModeChannels) + channel]; } } },  
 1406104: ($0, $1, $2, $3) => { const buffer = $0; const bufferLength = $1; const speakerModeChannels = $2; const frameFlag = $3; var data = HEAPF32.subarray((buffer / 4), (buffer / 4) + (bufferLength * speakerModeChannels)); const arrayOffset = speakerModeChannels * frameFlag * bufferLength; for (var channel = 0; channel < speakerModeChannels; channel++) { const offset = channel * bufferLength + arrayOffset; for (var sample = 0; sample < bufferLength; sample++) { Module.mOutputData[sample + offset] = data[(sample * speakerModeChannels) + channel]; } } },  
 1406635: () => { if (Module.mContext) { Module.mContext.suspend(); } },  
 1406691: () => { if (Module.mContext) { Module.mContext.resume(); } },  
 1406746: () => { debugger; }
};
function canvas_get_width() { return canvas.width; }
function canvas_get_height() { return canvas.height; }
function WriteFileJS(relPath,content) { const path = UTF8ToString(relPath); const data = UTF8ToString(content); try { const parts = path.split('/'); let current = ''; for (let i = 0; i < parts.length - 1; i++) { current += (i ? '/' : '') + parts[i]; if (!FS.analyzePath(current).exists) { FS.mkdir(current); } } FS.writeFile(path, data); } catch (e) { console.error("WriteFileJS failed:", e); } }
function WriteFileBinaryJS(relPath,data,size) { const path = UTF8ToString(relPath); const buffer = new Uint8Array(Module.HEAPU8.buffer, data, size); try { const parts = path.split('/'); let current = ''; for (let i = 0; i < parts.length - 1; i++) { current += (i ? '/' : '') + parts[i]; if (!FS.analyzePath(current).exists) { FS.mkdir(current); } } FS.writeFile(path, buffer); } catch (e) { console.error("WriteFileBinaryJS failed:", e); } }
function release_cursor_js() { if (document.pointerLockElement === Module['canvas']) { document.exitPointerLock(); } }
function lock_cursor_js() { if (Module['canvas']) { Module['canvas'].requestPointerLock(); } }
function ImGui_ImplSDL2_EmscriptenOpenURL(url) { url = url ? UTF8ToString(url) : null; if (url) window.open(url, '_blank'); }
var wasmImports = {
  /** @export */
  ImGui_ImplSDL2_EmscriptenOpenURL,
  /** @export */
  WriteFileBinaryJS,
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
  alSourceStop: _alSourceStop,
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
var dynCall_vi = Module['dynCall_vi'] = createExportWrapper('dynCall_vi', 2);
var dynCall_ii = Module['dynCall_ii'] = createExportWrapper('dynCall_ii', 2);
var dynCall_vii = Module['dynCall_vii'] = createExportWrapper('dynCall_vii', 3);
var dynCall_iii = Module['dynCall_iii'] = createExportWrapper('dynCall_iii', 3);
var dynCall_viif = Module['dynCall_viif'] = createExportWrapper('dynCall_viif', 4);
var dynCall_fiif = Module['dynCall_fiif'] = createExportWrapper('dynCall_fiif', 4);
var dynCall_iiii = Module['dynCall_iiii'] = createExportWrapper('dynCall_iiii', 4);
var dynCall_fi = Module['dynCall_fi'] = createExportWrapper('dynCall_fi', 2);
var dynCall_vif = Module['dynCall_vif'] = createExportWrapper('dynCall_vif', 3);
var dynCall_viii = Module['dynCall_viii'] = createExportWrapper('dynCall_viii', 4);
var dynCall_i = Module['dynCall_i'] = createExportWrapper('dynCall_i', 1);
var dynCall_vifii = Module['dynCall_vifii'] = createExportWrapper('dynCall_vifii', 5);
var dynCall_vifiiiii = Module['dynCall_vifiiiii'] = createExportWrapper('dynCall_vifiiiii', 8);
var dynCall_v = Module['dynCall_v'] = createExportWrapper('dynCall_v', 1);
var dynCall_viiif = Module['dynCall_viiif'] = createExportWrapper('dynCall_viiif', 5);
var dynCall_viiii = Module['dynCall_viiii'] = createExportWrapper('dynCall_viiii', 5);
var dynCall_iiiii = Module['dynCall_iiiii'] = createExportWrapper('dynCall_iiiii', 5);
var dynCall_iijii = Module['dynCall_iijii'] = createExportWrapper('dynCall_iijii', 5);
var dynCall_fiiijiijiijii = Module['dynCall_fiiijiijiijii'] = createExportWrapper('dynCall_fiiijiijiijii', 13);
var dynCall_viiiii = Module['dynCall_viiiii'] = createExportWrapper('dynCall_viiiii', 6);
var dynCall_iiiiiii = Module['dynCall_iiiiiii'] = createExportWrapper('dynCall_iiiiiii', 7);
var dynCall_viiiiiii = Module['dynCall_viiiiiii'] = createExportWrapper('dynCall_viiiiiii', 8);
var dynCall_iiiiii = Module['dynCall_iiiiii'] = createExportWrapper('dynCall_iiiiii', 6);
var dynCall_jiii = Module['dynCall_jiii'] = createExportWrapper('dynCall_jiii', 4);
var dynCall_vij = Module['dynCall_vij'] = createExportWrapper('dynCall_vij', 3);
var dynCall_viiiiiiii = Module['dynCall_viiiiiiii'] = createExportWrapper('dynCall_viiiiiiii', 9);
var dynCall_di = Module['dynCall_di'] = createExportWrapper('dynCall_di', 2);
var dynCall_viiiiii = Module['dynCall_viiiiii'] = createExportWrapper('dynCall_viiiiii', 7);
var dynCall_ff = Module['dynCall_ff'] = createExportWrapper('dynCall_ff', 2);
var dynCall_fff = Module['dynCall_fff'] = createExportWrapper('dynCall_fff', 3);
var dynCall_iiiiiiii = Module['dynCall_iiiiiiii'] = createExportWrapper('dynCall_iiiiiiii', 8);
var dynCall_iiiiiiiifii = Module['dynCall_iiiiiiiifii'] = createExportWrapper('dynCall_iiiiiiiifii', 11);
var dynCall_viiiffii = Module['dynCall_viiiffii'] = createExportWrapper('dynCall_viiiffii', 8);
var dynCall_iiiiiiiiii = Module['dynCall_iiiiiiiiii'] = createExportWrapper('dynCall_iiiiiiiiii', 10);
var dynCall_jiji = Module['dynCall_jiji'] = createExportWrapper('dynCall_jiji', 4);
var dynCall_ji = Module['dynCall_ji'] = createExportWrapper('dynCall_ji', 2);
var dynCall_iiif = Module['dynCall_iiif'] = createExportWrapper('dynCall_iiif', 4);
var dynCall_fiii = Module['dynCall_fiii'] = createExportWrapper('dynCall_fiii', 4);
var dynCall_iiifj = Module['dynCall_iiifj'] = createExportWrapper('dynCall_iiifj', 5);
var dynCall_fii = Module['dynCall_fii'] = createExportWrapper('dynCall_fii', 3);
var dynCall_iif = Module['dynCall_iif'] = createExportWrapper('dynCall_iif', 3);
var dynCall_iiji = Module['dynCall_iiji'] = createExportWrapper('dynCall_iiji', 4);
var dynCall_iij = Module['dynCall_iij'] = createExportWrapper('dynCall_iij', 3);
var dynCall_iiiff = Module['dynCall_iiiff'] = createExportWrapper('dynCall_iiiff', 5);
var dynCall_iijjifi = Module['dynCall_iijjifi'] = createExportWrapper('dynCall_iijjifi', 7);
var dynCall_iijj = Module['dynCall_iijj'] = createExportWrapper('dynCall_iijj', 4);
var dynCall_iijjifii = Module['dynCall_iijjifii'] = createExportWrapper('dynCall_iijjifii', 8);
var dynCall_iiff = Module['dynCall_iiff'] = createExportWrapper('dynCall_iiff', 4);
var dynCall_iifi = Module['dynCall_iifi'] = createExportWrapper('dynCall_iifi', 4);
var dynCall_iiffffffff = Module['dynCall_iiffffffff'] = createExportWrapper('dynCall_iiffffffff', 10);
var dynCall_iijji = Module['dynCall_iijji'] = createExportWrapper('dynCall_iijji', 5);
var dynCall_iijf = Module['dynCall_iijf'] = createExportWrapper('dynCall_iijf', 4);
var dynCall_iifff = Module['dynCall_iifff'] = createExportWrapper('dynCall_iifff', 5);
var dynCall_iiffi = Module['dynCall_iiffi'] = createExportWrapper('dynCall_iiffi', 5);
var dynCall_iiiffi = Module['dynCall_iiiffi'] = createExportWrapper('dynCall_iiiffi', 6);
var dynCall_iiifffii = Module['dynCall_iiifffii'] = createExportWrapper('dynCall_iiifffii', 8);
var dynCall_iiiifffffiii = Module['dynCall_iiiifffffiii'] = createExportWrapper('dynCall_iiiifffffiii', 12);
var dynCall_iiiffffii = Module['dynCall_iiiffffii'] = createExportWrapper('dynCall_iiiffffii', 9);
var dynCall_iiifffffii = Module['dynCall_iiifffffii'] = createExportWrapper('dynCall_iiifffffii', 10);
var dynCall_iiifffi = Module['dynCall_iiifffi'] = createExportWrapper('dynCall_iiifffi', 7);
var dynCall_viiiiiiffffff = Module['dynCall_viiiiiiffffff'] = createExportWrapper('dynCall_viiiiiiffffff', 13);
var dynCall_iiiiffi = Module['dynCall_iiiiffi'] = createExportWrapper('dynCall_iiiiffi', 7);
var dynCall_viiiiif = Module['dynCall_viiiiif'] = createExportWrapper('dynCall_viiiiif', 7);
var dynCall_iiiiiiiiiiiii = Module['dynCall_iiiiiiiiiiiii'] = createExportWrapper('dynCall_iiiiiiiiiiiii', 13);
var dynCall_iiiiiffi = Module['dynCall_iiiiiffi'] = createExportWrapper('dynCall_iiiiiffi', 8);
var dynCall_viiiiiiiifffii = Module['dynCall_viiiiiiiifffii'] = createExportWrapper('dynCall_viiiiiiiifffii', 14);
var dynCall_viiiiiiiiiiii = Module['dynCall_viiiiiiiiiiii'] = createExportWrapper('dynCall_viiiiiiiiiiii', 13);
var dynCall_viiiiiiiiifff = Module['dynCall_viiiiiiiiifff'] = createExportWrapper('dynCall_viiiiiiiiifff', 13);
var dynCall_iiiiiiiiiiii = Module['dynCall_iiiiiiiiiiii'] = createExportWrapper('dynCall_iiiiiiiiiiii', 12);
var dynCall_iiiiiiiii = Module['dynCall_iiiiiiiii'] = createExportWrapper('dynCall_iiiiiiiii', 9);
var dynCall_jii = Module['dynCall_jii'] = createExportWrapper('dynCall_jii', 3);
var dynCall_viiiiiiiiiii = Module['dynCall_viiiiiiiiiii'] = createExportWrapper('dynCall_viiiiiiiiiii', 12);
var dynCall_viiiiiiiii = Module['dynCall_viiiiiiiii'] = createExportWrapper('dynCall_viiiiiiiii', 10);
var dynCall_fiiii = Module['dynCall_fiiii'] = createExportWrapper('dynCall_fiiii', 5);
var dynCall_viifiii = Module['dynCall_viifiii'] = createExportWrapper('dynCall_viifiii', 7);
var dynCall_viiifiii = Module['dynCall_viiifiii'] = createExportWrapper('dynCall_viiifiii', 8);
var dynCall_vifi = Module['dynCall_vifi'] = createExportWrapper('dynCall_vifi', 4);
var dynCall_viiiiiiiiii = Module['dynCall_viiiiiiiiii'] = createExportWrapper('dynCall_viiiiiiiiii', 11);
var dynCall_vifiiii = Module['dynCall_vifiiii'] = createExportWrapper('dynCall_vifiiii', 7);
var dynCall_viji = Module['dynCall_viji'] = createExportWrapper('dynCall_viji', 4);
var dynCall_vidi = Module['dynCall_vidi'] = createExportWrapper('dynCall_vidi', 4);
var dynCall_vijii = Module['dynCall_vijii'] = createExportWrapper('dynCall_vijii', 5);
var dynCall_vidii = Module['dynCall_vidii'] = createExportWrapper('dynCall_vidii', 5);
var dynCall_iiiijij = Module['dynCall_iiiijij'] = createExportWrapper('dynCall_iiiijij', 7);
var dynCall_vffff = Module['dynCall_vffff'] = createExportWrapper('dynCall_vffff', 5);
var dynCall_vf = Module['dynCall_vf'] = createExportWrapper('dynCall_vf', 2);
var dynCall_vff = Module['dynCall_vff'] = createExportWrapper('dynCall_vff', 3);
var dynCall_vfi = Module['dynCall_vfi'] = createExportWrapper('dynCall_vfi', 3);
var dynCall_viff = Module['dynCall_viff'] = createExportWrapper('dynCall_viff', 4);
var dynCall_vifff = Module['dynCall_vifff'] = createExportWrapper('dynCall_vifff', 5);
var dynCall_viffff = Module['dynCall_viffff'] = createExportWrapper('dynCall_viffff', 6);
var dynCall_vfff = Module['dynCall_vfff'] = createExportWrapper('dynCall_vfff', 4);
var dynCall_viifi = Module['dynCall_viifi'] = createExportWrapper('dynCall_viifi', 5);
var dynCall_iidiiii = Module['dynCall_iidiiii'] = createExportWrapper('dynCall_iidiiii', 7);
var dynCall_j = Module['dynCall_j'] = createExportWrapper('dynCall_j', 1);
var dynCall_viijii = Module['dynCall_viijii'] = createExportWrapper('dynCall_viijii', 6);
var dynCall_iiiiij = Module['dynCall_iiiiij'] = createExportWrapper('dynCall_iiiiij', 6);
var dynCall_iiiiid = Module['dynCall_iiiiid'] = createExportWrapper('dynCall_iiiiid', 6);
var dynCall_iiiiiiiiiii = Module['dynCall_iiiiiiiiiii'] = createExportWrapper('dynCall_iiiiiiiiiii', 11);
var dynCall_jiiii = Module['dynCall_jiiii'] = createExportWrapper('dynCall_jiiii', 5);
var dynCall_diii = Module['dynCall_diii'] = createExportWrapper('dynCall_diii', 4);
var dynCall_viiiiiiiiiiiiiii = Module['dynCall_viiiiiiiiiiiiiii'] = createExportWrapper('dynCall_viiiiiiiiiiiiiii', 16);
var dynCall_iiiiijj = Module['dynCall_iiiiijj'] = createExportWrapper('dynCall_iiiiijj', 7);
var dynCall_iiiiiijj = Module['dynCall_iiiiiijj'] = createExportWrapper('dynCall_iiiiiijj', 8);
var _asyncify_start_unwind = createExportWrapper('asyncify_start_unwind', 1);
var _asyncify_stop_unwind = createExportWrapper('asyncify_stop_unwind', 0);
var _asyncify_start_rewind = createExportWrapper('asyncify_start_rewind', 1);
var _asyncify_stop_rewind = createExportWrapper('asyncify_stop_rewind', 0);

function invoke_viiii(index,a1,a2,a3,a4) {
  var sp = stackSave();
  try {
    dynCall_viiii(index,a1,a2,a3,a4);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iii(index,a1,a2) {
  var sp = stackSave();
  try {
    return dynCall_iii(index,a1,a2);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiii(index,a1,a2,a3,a4) {
  var sp = stackSave();
  try {
    return dynCall_iiiii(index,a1,a2,a3,a4);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_v(index) {
  var sp = stackSave();
  try {
    dynCall_v(index);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiii(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    return dynCall_iiii(index,a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_j(index) {
  var sp = stackSave();
  try {
    return dynCall_j(index);
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
    return dynCall_ji(index,a1);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
    return 0n;
  }
}

function invoke_ii(index,a1) {
  var sp = stackSave();
  try {
    return dynCall_ii(index,a1);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_vii(index,a1,a2) {
  var sp = stackSave();
  try {
    dynCall_vii(index,a1,a2);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viii(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    dynCall_viii(index,a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_vi(index,a1) {
  var sp = stackSave();
  try {
    dynCall_vi(index,a1);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiii(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    return dynCall_iiiiii(index,a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiiii(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    dynCall_viiiii(index,a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiii(index,a1,a2,a3,a4,a5,a6) {
  var sp = stackSave();
  try {
    return dynCall_iiiiiii(index,a1,a2,a3,a4,a5,a6);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viijii(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    dynCall_viijii(index,a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiij(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    return dynCall_iiiiij(index,a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiid(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    return dynCall_iiiiid(index,a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiiii(index,a1,a2,a3,a4,a5,a6,a7) {
  var sp = stackSave();
  try {
    return dynCall_iiiiiiii(index,a1,a2,a3,a4,a5,a6,a7);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) {
  var sp = stackSave();
  try {
    return dynCall_iiiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_jiiii(index,a1,a2,a3,a4) {
  var sp = stackSave();
  try {
    return dynCall_jiiii(index,a1,a2,a3,a4);
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
    return dynCall_iiiiiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_fiii(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    return dynCall_fiii(index,a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_diii(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    return dynCall_diii(index,a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_i(index) {
  var sp = stackSave();
  try {
    return dynCall_i(index);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiiiiii(index,a1,a2,a3,a4,a5,a6,a7) {
  var sp = stackSave();
  try {
    dynCall_viiiiiii(index,a1,a2,a3,a4,a5,a6,a7);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11) {
  var sp = stackSave();
  try {
    return dynCall_iiiiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) {
  var sp = stackSave();
  try {
    dynCall_viiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiiiiiiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15) {
  var sp = stackSave();
  try {
    dynCall_viiiiiiiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viif(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    dynCall_viif(index,a1,a2,a3);
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

