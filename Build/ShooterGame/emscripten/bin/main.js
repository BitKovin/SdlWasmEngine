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
// include: C:\Users\bogda_\AppData\Local\Temp\tmp5z6hm23j.js

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
Module['FS_createPath']("/GameData", "PSOs", true, true);
Module['FS_createPath']("/GameData", "animations", true, true);
Module['FS_createPath']("/GameData/animations", "npc", true, true);
Module['FS_createPath']("/GameData/animations", "player", true, true);
Module['FS_createPath']("/GameData/animations/player", "body", true, true);
Module['FS_createPath']("/GameData", "behaviourTrees", true, true);
Module['FS_createPath']("/GameData", "env", true, true);
Module['FS_createPath']("/GameData/env", "example", true, true);
Module['FS_createPath']("/GameData", "fonts", true, true);
Module['FS_createPath']("/GameData", "maps", true, true);
Module['FS_createPath']("/GameData/maps", "autosave", true, true);
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
Module['FS_createPath']("/GameData/models/player/body", "textures", true, true);
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
Module['FS_createPath']("/GameData/textures", "levelPlans", true, true);
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
    loadPackage({"files": [{"filename": "/GameData/PSOs/pso_cache.json", "start": 0, "end": 6054}, {"filename": "/GameData/animations/npc/WrithingInPain.fbx", "start": 6054, "end": 1690422}, {"filename": "/GameData/animations/npc/caution_idle.glb", "start": 1690422, "end": 3555018}, {"filename": "/GameData/animations/npc/idle.glb", "start": 3555018, "end": 5367498}, {"filename": "/GameData/animations/npc/inPain.glb", "start": 5367498, "end": 7288170}, {"filename": "/GameData/animations/npc/run.glb", "start": 7288170, "end": 9116530}, {"filename": "/GameData/animations/npc/standUp.glb", "start": 9116530, "end": 11455810}, {"filename": "/GameData/animations/npc/walk.glb", "start": 11455810, "end": 12196438}, {"filename": "/GameData/animations/player/body/idle.glb", "start": 12196438, "end": 14008918}, {"filename": "/GameData/animations/player/body/run_f.glb", "start": 14008918, "end": 15837278}, {"filename": "/GameData/arms.glb", "start": 15837278, "end": 16017042}, {"filename": "/GameData/behaviourTrees/general.bt", "start": 16017042, "end": 16100684}, {"filename": "/GameData/behaviourTrees/test", "start": 16100684, "end": 16101782}, {"filename": "/GameData/behaviourTrees/test2", "start": 16101782, "end": 16107690}, {"filename": "/GameData/behaviourTrees/test3", "start": 16107690, "end": 16109599}, {"filename": "/GameData/cat.png", "start": 16109599, "end": 16359037}, {"filename": "/GameData/cube.mtl", "start": 16359037, "end": 16359090}, {"filename": "/GameData/env/example/skybox2_overcast_cube_bk.png", "start": 16359090, "end": 16852560}, {"filename": "/GameData/env/example/skybox2_overcast_cube_dn.png", "start": 16852560, "end": 17634228}, {"filename": "/GameData/env/example/skybox2_overcast_cube_ft.png", "start": 17634228, "end": 18049013}, {"filename": "/GameData/env/example/skybox2_overcast_cube_lf.png", "start": 18049013, "end": 18485624}, {"filename": "/GameData/env/example/skybox2_overcast_cube_rt.png", "start": 18485624, "end": 18948092}, {"filename": "/GameData/env/example/skybox2_overcast_cube_up.png", "start": 18948092, "end": 19270359}, {"filename": "/GameData/env/notes.txt", "start": 19270359, "end": 19270447}, {"filename": "/GameData/env/skybox1_night_cube.png", "start": 19270447, "end": 21359461}, {"filename": "/GameData/env/skybox2_overcast_cube_bk.png", "start": 21359461, "end": 22039563}, {"filename": "/GameData/env/skybox2_overcast_cube_dn.png", "start": 22039563, "end": 23026157}, {"filename": "/GameData/env/skybox2_overcast_cube_dn_.png", "start": 23026157, "end": 24012014}, {"filename": "/GameData/env/skybox2_overcast_cube_ft.png", "start": 24012014, "end": 24617886}, {"filename": "/GameData/env/skybox2_overcast_cube_lf.png", "start": 24617886, "end": 25265725}, {"filename": "/GameData/env/skybox2_overcast_cube_rt.png", "start": 25265725, "end": 25915848}, {"filename": "/GameData/env/skybox2_overcast_cube_up.png", "start": 25915848, "end": 26410458}, {"filename": "/GameData/env/skybox2_overcast_cube_up_.png", "start": 26410458, "end": 26905001}, {"filename": "/GameData/env/skybox2_overcast_night_cube.png", "start": 26905001, "end": 28229244}, {"filename": "/GameData/env/skybox2_overcast_night_cube_bk.png", "start": 28229244, "end": 29387628}, {"filename": "/GameData/env/skybox2_overcast_night_cube_dn.png", "start": 29387628, "end": 30895105}, {"filename": "/GameData/env/skybox2_overcast_night_cube_ft.png", "start": 30895105, "end": 31968811}, {"filename": "/GameData/env/skybox2_overcast_night_cube_lf.png", "start": 31968811, "end": 33060005}, {"filename": "/GameData/env/skybox2_overcast_night_cube_rt.png", "start": 33060005, "end": 34233014}, {"filename": "/GameData/env/skybox2_overcast_night_cube_up.png", "start": 34233014, "end": 35048572}, {"filename": "/GameData/fonts/Font Awesome 6 Free-Regular-400.ttf", "start": 35048572, "end": 35124312}, {"filename": "/GameData/fonts/Kingthings_Calligraphica_2.ttf", "start": 35124312, "end": 35154116}, {"filename": "/GameData/fonts/fa-regular-400.ttf", "start": 35154116, "end": 35222180}, {"filename": "/GameData/maps/Level.bsp", "start": 35222180, "end": 35284256}, {"filename": "/GameData/maps/autosave/l1_town.1.map", "start": 35284256, "end": 35288946}, {"filename": "/GameData/maps/autosave/l1_town.10.map", "start": 35288946, "end": 35456158}, {"filename": "/GameData/maps/autosave/l1_town.2.map", "start": 35456158, "end": 35481569}, {"filename": "/GameData/maps/autosave/l1_town.3.map", "start": 35481569, "end": 35492603}, {"filename": "/GameData/maps/autosave/l1_town.4.map", "start": 35492603, "end": 35520317}, {"filename": "/GameData/maps/autosave/l1_town.5.map", "start": 35520317, "end": 35609022}, {"filename": "/GameData/maps/autosave/l1_town.6.map", "start": 35609022, "end": 35721065}, {"filename": "/GameData/maps/autosave/l1_town.7.map", "start": 35721065, "end": 35847306}, {"filename": "/GameData/maps/autosave/l1_town.8.map", "start": 35847306, "end": 36014811}, {"filename": "/GameData/maps/autosave/l1_town.9.map", "start": 36014811, "end": 36183079}, {"filename": "/GameData/maps/autosave/lvl1_proto.1.map", "start": 36183079, "end": 36184965}, {"filename": "/GameData/maps/autosave/lvl1_proto.2.map", "start": 36184965, "end": 36333783}, {"filename": "/GameData/maps/autosave/lvl1_proto.3.map", "start": 36333783, "end": 36487694}, {"filename": "/GameData/maps/autosave/lvl1_proto.4.map", "start": 36487694, "end": 36641945}, {"filename": "/GameData/maps/autosave/lvl1_proto.5.map", "start": 36641945, "end": 36803025}, {"filename": "/GameData/maps/autosave/lvl1_proto.6.map", "start": 36803025, "end": 37008052}, {"filename": "/GameData/maps/autosave/lvl1_proto.7.map", "start": 37008052, "end": 37223024}, {"filename": "/GameData/maps/autosave/lvl1_proto.8.map", "start": 37223024, "end": 37505919}, {"filename": "/GameData/maps/autosave/proto2.1.map", "start": 37505919, "end": 37511355}, {"filename": "/GameData/maps/autosave/proto2.2.map", "start": 37511355, "end": 37523238}, {"filename": "/GameData/maps/autosave/proto2.3.map", "start": 37523238, "end": 37535137}, {"filename": "/GameData/maps/autosave/proto2.4.map", "start": 37535137, "end": 37549126}, {"filename": "/GameData/maps/autosave/proto2.5.map", "start": 37549126, "end": 37563645}, {"filename": "/GameData/maps/autosave/test.1.map", "start": 37563645, "end": 37579596}, {"filename": "/GameData/maps/autosave/test.10.map", "start": 37579596, "end": 37671284}, {"filename": "/GameData/maps/autosave/test.11.map", "start": 37671284, "end": 37778290}, {"filename": "/GameData/maps/autosave/test2.1.map", "start": 37778290, "end": 38434858}, {"filename": "/GameData/maps/blank.autosave.map", "start": 38434858, "end": 38440458}, {"filename": "/GameData/maps/blank.bak", "start": 38440458, "end": 38446058}, {"filename": "/GameData/maps/blank.bsp", "start": 38446058, "end": 38550518}, {"filename": "/GameData/maps/blank.bsp.nav", "start": 38550518, "end": 38550821}, {"filename": "/GameData/maps/blank.map", "start": 38550821, "end": 38556439}, {"filename": "/GameData/maps/blank.srf", "start": 38556439, "end": 38557922}, {"filename": "/GameData/maps/l1_town.autosave.map", "start": 38557922, "end": 38751746}, {"filename": "/GameData/maps/l1_town.bak", "start": 38751746, "end": 38945164}, {"filename": "/GameData/maps/l1_town.bsp.nav", "start": 38945164, "end": 39300281}, {"filename": "/GameData/maps/l1_town.map", "start": 39300281, "end": 39494105}, {"filename": "/GameData/maps/l1_town.srf", "start": 39494105, "end": 39594780}, {"filename": "/GameData/maps/l1_town.zip", "start": 39594780, "end": 71387016}, {"filename": "/GameData/maps/lvl1_proto.autosave.map", "start": 71387016, "end": 71725777}, {"filename": "/GameData/maps/lvl1_proto.bak", "start": 71725777, "end": 72064514}, {"filename": "/GameData/maps/lvl1_proto.bsp.nav", "start": 72064514, "end": 72335231}, {"filename": "/GameData/maps/lvl1_proto.bsp.svd", "start": 72335231, "end": 74634671}, {"filename": "/GameData/maps/lvl1_proto.map", "start": 74634671, "end": 74973432}, {"filename": "/GameData/maps/lvl1_proto.prt", "start": 74973432, "end": 75364938}, {"filename": "/GameData/maps/lvl1_proto.srf", "start": 75364938, "end": 75505215}, {"filename": "/GameData/maps/lvl1_proto.zip", "start": 75505215, "end": 77111994}, {"filename": "/GameData/maps/proto1.bak", "start": 77111994, "end": 77113508}, {"filename": "/GameData/maps/proto1.map", "start": 77113508, "end": 77115022}, {"filename": "/GameData/maps/proto2.autosave.map", "start": 77115022, "end": 77125795}, {"filename": "/GameData/maps/proto2.bak", "start": 77125795, "end": 77140314}, {"filename": "/GameData/maps/proto2.bsp", "start": 77140314, "end": 79978402}, {"filename": "/GameData/maps/proto2.map", "start": 79978402, "end": 79993190}, {"filename": "/GameData/maps/proto2.srf", "start": 79993190, "end": 80004482}, {"filename": "/GameData/maps/proto2/lm_0000.tga", "start": 80004482, "end": 80201108}, {"filename": "/GameData/maps/proto2/lm_0001.tga", "start": 80201108, "end": 80397734}, {"filename": "/GameData/maps/t.autosave.map", "start": 80397734, "end": 80398365}, {"filename": "/GameData/maps/t.bak", "start": 80398365, "end": 80399000}, {"filename": "/GameData/maps/t.map", "start": 80399000, "end": 80399631}, {"filename": "/GameData/maps/test.autosave.map", "start": 80399631, "end": 80509900}, {"filename": "/GameData/maps/test.bak", "start": 80509900, "end": 80620299}, {"filename": "/GameData/maps/test.bsp.nav", "start": 80620299, "end": 80876779}, {"filename": "/GameData/maps/test.map", "start": 80876779, "end": 80987048}, {"filename": "/GameData/maps/test.mtl", "start": 80987048, "end": 80987325}, {"filename": "/GameData/maps/test.prt", "start": 80987325, "end": 81096793}, {"filename": "/GameData/maps/test.srf", "start": 81096793, "end": 81164812}, {"filename": "/GameData/maps/test.zip", "start": 81164812, "end": 82038164}, {"filename": "/GameData/maps/test2.bsp", "start": 82038164, "end": 85151732}, {"filename": "/GameData/maps/test2.lin", "start": 85151732, "end": 85151809}, {"filename": "/GameData/maps/test2.map", "start": 85151809, "end": 85809684}, {"filename": "/GameData/maps/test2.mtl", "start": 85809684, "end": 85810379}, {"filename": "/GameData/maps/test2.srf", "start": 85810379, "end": 86484382}, {"filename": "/GameData/maps/test3.bsp", "start": 86484382, "end": 95390458}, {"filename": "/GameData/maps/test3.map", "start": 95390458, "end": 95451588}, {"filename": "/GameData/maps/test3.srf", "start": 95451588, "end": 95507730}, {"filename": "/GameData/maps/test4.bak", "start": 95507730, "end": 95512963}, {"filename": "/GameData/maps/test4.map", "start": 95512963, "end": 95522288}, {"filename": "/GameData/maps/test_interior.autosave.map", "start": 95522288, "end": 95526728}, {"filename": "/GameData/maps/test_interior.bak", "start": 95526728, "end": 95531049}, {"filename": "/GameData/maps/test_interior.bsp", "start": 95531049, "end": 95611293}, {"filename": "/GameData/maps/test_interior.bsp.nav", "start": 95611293, "end": 95613792}, {"filename": "/GameData/maps/test_interior.bsp.svd", "start": 95613792, "end": 95661120}, {"filename": "/GameData/maps/test_interior.map", "start": 95661120, "end": 95665560}, {"filename": "/GameData/maps/test_interior.srf", "start": 95665560, "end": 95667958}, {"filename": "/GameData/models/cube.mtl", "start": 95667958, "end": 95668011}, {"filename": "/GameData/models/cube.obj", "start": 95668011, "end": 95668969}, {"filename": "/GameData/models/effects/explosion.glb", "start": 95668969, "end": 95846861}, {"filename": "/GameData/models/enemies/cultist_base.blend", "start": 95846861, "end": 100987504}, {"filename": "/GameData/models/enemies/dog/dog.fbx", "start": 100987504, "end": 101323516}, {"filename": "/GameData/models/enemies/dog/dog.glb", "start": 101323516, "end": 101452128}, {"filename": "/GameData/models/enemies/dog/dog.glb.skmm", "start": 101452128, "end": 101470750}, {"filename": "/GameData/models/enemies/humanAxe/humanAxe.glb", "start": 101470750, "end": 101905658}, {"filename": "/GameData/models/enemies/humanAxe/humanAxe.glb.skmm", "start": 101905658, "end": 101917116}, {"filename": "/GameData/models/enemies/humanAxe/humanAxe1.glb", "start": 101917116, "end": 102352252}, {"filename": "/GameData/models/enemies/humanGun/humanGun.glb", "start": 102352252, "end": 106039576}, {"filename": "/GameData/models/enemies/humanGun/humanGun.glb.skmm", "start": 106039576, "end": 106051175}, {"filename": "/GameData/models/enemies/zombie/zombie.glb", "start": 106051175, "end": 106588579}, {"filename": "/GameData/models/enemies/zombie/zombie.glb.skmm", "start": 106588579, "end": 106600037}, {"filename": "/GameData/models/engine/invSphere.mtl", "start": 106600037, "end": 106600094}, {"filename": "/GameData/models/engine/invSphere.obj", "start": 106600094, "end": 106606278}, {"filename": "/GameData/models/engine/widgetPlane.glb", "start": 106606278, "end": 106607586}, {"filename": "/GameData/models/enviroment/door.glb", "start": 106607586, "end": 111928554}, {"filename": "/GameData/models/npc/base.glb", "start": 111928554, "end": 113083706}, {"filename": "/GameData/models/npc/base.glb.skmm", "start": 113083706, "end": 113095159}, {"filename": "/GameData/models/npc/guard.glb", "start": 113095159, "end": 117981603}, {"filename": "/GameData/models/npc_base.mtl", "start": 117981603, "end": 117981844}, {"filename": "/GameData/models/npc_base.obj", "start": 117981844, "end": 117984182}, {"filename": "/GameData/models/pickups/12Cal.png", "start": 117984182, "end": 118131378}, {"filename": "/GameData/models/pickups/38Cal.png", "start": 118131378, "end": 118559199}, {"filename": "/GameData/models/pickups/762Cal.png", "start": 118559199, "end": 118702571}, {"filename": "/GameData/models/pickups/cannonAmmo.obj", "start": 118702571, "end": 118703820}, {"filename": "/GameData/models/pickups/pistolAmmo.obj", "start": 118703820, "end": 118705077}, {"filename": "/GameData/models/pickups/shotgunAmmo.obj", "start": 118705077, "end": 118706327}, {"filename": "/GameData/models/player/arms.glb", "start": 118706327, "end": 119060475}, {"filename": "/GameData/models/player/arms2.glb", "start": 119060475, "end": 119665219}, {"filename": "/GameData/models/player/arms_p.glb", "start": 119665219, "end": 120106903}, {"filename": "/GameData/models/player/bike/bike.glb", "start": 120106903, "end": 125558779}, {"filename": "/GameData/models/player/bike/textures/body.png", "start": 125558779, "end": 125795460}, {"filename": "/GameData/models/player/bike/textures/front.png", "start": 125795460, "end": 126267211}, {"filename": "/GameData/models/player/bike/textures/wheels.png", "start": 126267211, "end": 127394015}, {"filename": "/GameData/models/player/body/player_body.glb", "start": 127394015, "end": 131816227}, {"filename": "/GameData/models/player/body/textures/coat.png", "start": 131816227, "end": 134323847}, {"filename": "/GameData/models/player/body/textures/pants.png", "start": 134323847, "end": 137898245}, {"filename": "/GameData/models/player/body/textures/shirt.png", "start": 137898245, "end": 138089996}, {"filename": "/GameData/models/player/body/textures/shoes.png", "start": 138089996, "end": 139386871}, {"filename": "/GameData/models/player/weapons/arms.glb", "start": 139386871, "end": 139741019}, {"filename": "/GameData/models/player/weapons/arms2.glb", "start": 139741019, "end": 140345763}, {"filename": "/GameData/models/player/weapons/cane/cane.glb", "start": 140345763, "end": 140729079}, {"filename": "/GameData/models/player/weapons/cane/cane.glb.skmm", "start": 140729079, "end": 140730039}, {"filename": "/GameData/models/player/weapons/cane/cane.mtl", "start": 140730039, "end": 140730278}, {"filename": "/GameData/models/player/weapons/cane/cane.obj", "start": 140730278, "end": 140758044}, {"filename": "/GameData/models/player/weapons/cane/cane.png", "start": 140758044, "end": 140847793}, {"filename": "/GameData/models/player/weapons/cannon/cannon.glb", "start": 140847793, "end": 142897665}, {"filename": "/GameData/models/player/weapons/leftHand/empty.glb", "start": 142897665, "end": 143201669}, {"filename": "/GameData/models/player/weapons/mpsd/mpsd.glb", "start": 143201669, "end": 146002305}, {"filename": "/GameData/models/player/weapons/mpsd/mpsd_tp.glb", "start": 146002305, "end": 148811889}, {"filename": "/GameData/models/player/weapons/pistol/glock.glb", "start": 148811889, "end": 149532057}, {"filename": "/GameData/models/player/weapons/pistol/muzzle_t.png", "start": 149532057, "end": 149541465}, {"filename": "/GameData/models/player/weapons/pistol/muzzle_t_em.png", "start": 149541465, "end": 149550873}, {"filename": "/GameData/models/player/weapons/pistol/pistol.glb", "start": 149550873, "end": 149821621}, {"filename": "/GameData/models/player/weapons/pistol/pistol.png", "start": 149821621, "end": 149913384}, {"filename": "/GameData/models/player/weapons/pistol/pistol_double_tp.glb", "start": 149913384, "end": 150992896}, {"filename": "/GameData/models/player/weapons/pistol/pistol_tp.glb", "start": 150992896, "end": 151748368}, {"filename": "/GameData/models/player/weapons/revolver/revolver.glb", "start": 151748368, "end": 152997016}, {"filename": "/GameData/models/player/weapons/shotgun/shotgun.glb", "start": 152997016, "end": 153663240}, {"filename": "/GameData/models/player/weapons/sniper/sniper.glb", "start": 153663240, "end": 158827284}, {"filename": "/GameData/models/player/weapons/sniper/sniper_tp.glb", "start": 158827284, "end": 169411712}, {"filename": "/GameData/models/player/weapons/stg44/stg44.glb", "start": 169411712, "end": 173236996}, {"filename": "/GameData/models/player/weapons/sword/sword.glb", "start": 173236996, "end": 173851156}, {"filename": "/GameData/models/player/weapons/swords/swords_tp.glb", "start": 173851156, "end": 174189448}, {"filename": "/GameData/models/player/weapons/swords/swords_tp.glb.skmm", "start": 174189448, "end": 174192389}, {"filename": "/GameData/models/player/weapons/tommy/tommy.glb", "start": 174192389, "end": 176129929}, {"filename": "/GameData/models/test.png", "start": 176129929, "end": 176130750}, {"filename": "/GameData/models/tools/door.mtl", "start": 176130750, "end": 176130989}, {"filename": "/GameData/models/tools/door.obj", "start": 176130989, "end": 176199585}, {"filename": "/GameData/models/tools/door_texture.png", "start": 176199585, "end": 181474183}, {"filename": "/GameData/models/tools/sittingPos.mtl", "start": 181474183, "end": 181474634}, {"filename": "/GameData/models/tools/sittingPos.obj", "start": 181474634, "end": 181899193}, {"filename": "/GameData/models/weapons/glock.glb", "start": 181899193, "end": 182381141}, {"filename": "/GameData/scripts/common.shader", "start": 182381141, "end": 182381954}, {"filename": "/GameData/scripts/skies.shader", "start": 182381954, "end": 182383620}, {"filename": "/GameData/shaders/ShaderCompilationAutomation.exe", "start": 182383620, "end": 182562924}, {"filename": "/GameData/shaders/compiled/linux/gl/bsp/fs_bsp.bin", "start": 182562924, "end": 182568375}, {"filename": "/GameData/shaders/compiled/linux/gl/bsp/fs_bsp_cube.bin", "start": 182568375, "end": 182568800}, {"filename": "/GameData/shaders/compiled/linux/gl/bsp/fs_bsp_empty.bin", "start": 182568800, "end": 182568880}, {"filename": "/GameData/shaders/compiled/linux/gl/bsp/vs_bsp.bin", "start": 182568880, "end": 182571175}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_bsp.bin", "start": 182571175, "end": 182576626}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_bsp_cube.bin", "start": 182576626, "end": 182577051}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_bsp_empty.bin", "start": 182577051, "end": 182577131}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_copy.bin", "start": 182577131, "end": 182577301}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_copy_depth.bin", "start": 182577301, "end": 182577470}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_customId.bin", "start": 182577470, "end": 182578253}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_default.bin", "start": 182578253, "end": 182586002}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_default_simple.bin", "start": 182586002, "end": 182589797}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_empty.bin", "start": 182589797, "end": 182589877}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_fxaa_simple.bin", "start": 182589877, "end": 182593335}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_mask.bin", "start": 182593335, "end": 182593596}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_motionBlur.bin", "start": 182593596, "end": 182595620}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_motionBlur_apply.bin", "start": 182595620, "end": 182596032}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_postprocessing.bin", "start": 182596032, "end": 182602849}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_resolve_depth_msaa.bin", "start": 182602849, "end": 182603425}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_solidRed.bin", "start": 182603425, "end": 182603505}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_ui_flatcolor.bin", "start": 182603505, "end": 182603608}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_ui_textured.bin", "start": 182603608, "end": 182603948}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_unlit.bin", "start": 182603948, "end": 182604360}, {"filename": "/GameData/shaders/compiled/linux/gl/fs_unlit_rect.bin", "start": 182604360, "end": 182604858}, {"filename": "/GameData/shaders/compiled/linux/gl/game/fs_debuff.bin", "start": 182604858, "end": 182607335}, {"filename": "/GameData/shaders/compiled/linux/gl/game/fs_inventory_menu_sphere.bin", "start": 182607335, "end": 182607478}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_blendmask.bin", "start": 182607478, "end": 182607730}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_blur.bin", "start": 182607730, "end": 182610855}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_color.bin", "start": 182610855, "end": 182610937}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_colormatrix.bin", "start": 182610937, "end": 182611566}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_creation.bin", "start": 182611566, "end": 182611646}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_dropshadow.bin", "start": 182611646, "end": 182611952}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_gradient.bin", "start": 182611952, "end": 182615998}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_passthrough.bin", "start": 182615998, "end": 182616159}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/fs_rmlui_texture.bin", "start": 182616159, "end": 182616351}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/vs_rmlui.bin", "start": 182616351, "end": 182616755}, {"filename": "/GameData/shaders/compiled/linux/gl/rmlui/vs_rmlui_passthrough.bin", "start": 182616755, "end": 182617181}, {"filename": "/GameData/shaders/compiled/linux/gl/ui/fs_progressBar.bin", "start": 182617181, "end": 182617782}, {"filename": "/GameData/shaders/compiled/linux/gl/vs_bsp.bin", "start": 182617782, "end": 182620077}, {"filename": "/GameData/shaders/compiled/linux/gl/vs_default.bin", "start": 182620077, "end": 182624743}, {"filename": "/GameData/shaders/compiled/linux/gl/vs_fullscreen.bin", "start": 182624743, "end": 182624978}, {"filename": "/GameData/shaders/compiled/linux/gl/vs_instanced_billboard.bin", "start": 182624978, "end": 182626180}, {"filename": "/GameData/shaders/compiled/linux/gl/vs_ui.bin", "start": 182626180, "end": 182626534}, {"filename": "/GameData/shaders/compiled/linux/spirv/bsp/fs_bsp.bin", "start": 182626534, "end": 182635413}, {"filename": "/GameData/shaders/compiled/linux/spirv/bsp/fs_bsp_cube.bin", "start": 182635413, "end": 182636990}, {"filename": "/GameData/shaders/compiled/linux/spirv/bsp/fs_bsp_empty.bin", "start": 182636990, "end": 182637352}, {"filename": "/GameData/shaders/compiled/linux/spirv/bsp/vs_bsp.bin", "start": 182637352, "end": 182641066}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_bsp.bin", "start": 182641066, "end": 182649945}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_bsp_cube.bin", "start": 182649945, "end": 182651522}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_bsp_empty.bin", "start": 182651522, "end": 182651884}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_copy.bin", "start": 182651884, "end": 182652730}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_copy_depth.bin", "start": 182652730, "end": 182653599}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_customId.bin", "start": 182653599, "end": 182655464}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_default.bin", "start": 182655464, "end": 182664631}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_default_simple.bin", "start": 182664631, "end": 182670724}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_empty.bin", "start": 182670724, "end": 182671086}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_fxaa_simple.bin", "start": 182671086, "end": 182674753}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_mask.bin", "start": 182674753, "end": 182675639}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_motionBlur.bin", "start": 182675639, "end": 182679857}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_motionBlur_apply.bin", "start": 182679857, "end": 182681217}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_postprocessing.bin", "start": 182681217, "end": 182690269}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_resolve_depth_msaa.bin", "start": 182690269, "end": 182691796}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_solidRed.bin", "start": 182691796, "end": 182692174}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_ui_flatcolor.bin", "start": 182692174, "end": 182692758}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_ui_textured.bin", "start": 182692758, "end": 182693974}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_unlit.bin", "start": 182693974, "end": 182695241}, {"filename": "/GameData/shaders/compiled/linux/spirv/fs_unlit_rect.bin", "start": 182695241, "end": 182696718}, {"filename": "/GameData/shaders/compiled/linux/spirv/game/fs_debuff.bin", "start": 182696718, "end": 182699683}, {"filename": "/GameData/shaders/compiled/linux/spirv/game/fs_inventory_menu_sphere.bin", "start": 182699683, "end": 182700077}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_blendmask.bin", "start": 182700077, "end": 182701117}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_blur.bin", "start": 182701117, "end": 182703745}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_color.bin", "start": 182703745, "end": 182704151}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_colormatrix.bin", "start": 182704151, "end": 182706121}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_creation.bin", "start": 182706121, "end": 182706483}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_dropshadow.bin", "start": 182706483, "end": 182707734}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_gradient.bin", "start": 182707734, "end": 182710962}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_passthrough.bin", "start": 182710962, "end": 182711717}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/fs_rmlui_texture.bin", "start": 182711717, "end": 182712580}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/vs_rmlui.bin", "start": 182712580, "end": 182713894}, {"filename": "/GameData/shaders/compiled/linux/spirv/rmlui/vs_rmlui_passthrough.bin", "start": 182713894, "end": 182715296}, {"filename": "/GameData/shaders/compiled/linux/spirv/ui/fs_progressBar.bin", "start": 182715296, "end": 182717205}, {"filename": "/GameData/shaders/compiled/linux/spirv/vs_bsp.bin", "start": 182717205, "end": 182720919}, {"filename": "/GameData/shaders/compiled/linux/spirv/vs_default.bin", "start": 182720919, "end": 182727630}, {"filename": "/GameData/shaders/compiled/linux/spirv/vs_fullscreen.bin", "start": 182727630, "end": 182728368}, {"filename": "/GameData/shaders/compiled/linux/spirv/vs_instanced_billboard.bin", "start": 182728368, "end": 182731204}, {"filename": "/GameData/shaders/compiled/linux/spirv/vs_ui.bin", "start": 182731204, "end": 182732530}, {"filename": "/GameData/shaders/compiled/web/gles/bsp/fs_bsp.bin", "start": 182732530, "end": 182738159}, {"filename": "/GameData/shaders/compiled/web/gles/bsp/fs_bsp_cube.bin", "start": 182738159, "end": 182738713}, {"filename": "/GameData/shaders/compiled/web/gles/bsp/fs_bsp_empty.bin", "start": 182738713, "end": 182738872}, {"filename": "/GameData/shaders/compiled/web/gles/bsp/vs_bsp.bin", "start": 182738872, "end": 182741367}, {"filename": "/GameData/shaders/compiled/web/gles/fs_bsp.bin", "start": 182741367, "end": 182746996}, {"filename": "/GameData/shaders/compiled/web/gles/fs_bsp_cube.bin", "start": 182746996, "end": 182747550}, {"filename": "/GameData/shaders/compiled/web/gles/fs_bsp_empty.bin", "start": 182747550, "end": 182747709}, {"filename": "/GameData/shaders/compiled/web/gles/fs_copy.bin", "start": 182747709, "end": 182748003}, {"filename": "/GameData/shaders/compiled/web/gles/fs_copy_depth.bin", "start": 182748003, "end": 182748261}, {"filename": "/GameData/shaders/compiled/web/gles/fs_customId.bin", "start": 182748261, "end": 182749136}, {"filename": "/GameData/shaders/compiled/web/gles/fs_default.bin", "start": 182749136, "end": 182757047}, {"filename": "/GameData/shaders/compiled/web/gles/fs_default_simple.bin", "start": 182757047, "end": 182761009}, {"filename": "/GameData/shaders/compiled/web/gles/fs_empty.bin", "start": 182761009, "end": 182761168}, {"filename": "/GameData/shaders/compiled/web/gles/fs_fxaa_simple.bin", "start": 182761168, "end": 182765115}, {"filename": "/GameData/shaders/compiled/web/gles/fs_mask.bin", "start": 182765115, "end": 182765425}, {"filename": "/GameData/shaders/compiled/web/gles/fs_motionBlur.bin", "start": 182765425, "end": 182767601}, {"filename": "/GameData/shaders/compiled/web/gles/fs_motionBlur_apply.bin", "start": 182767601, "end": 182768102}, {"filename": "/GameData/shaders/compiled/web/gles/fs_postprocessing.bin", "start": 182768102, "end": 182775437}, {"filename": "/GameData/shaders/compiled/web/gles/fs_resolve_depth_msaa.bin", "start": 182775437, "end": 182775540}, {"filename": "/GameData/shaders/compiled/web/gles/fs_solidRed.bin", "start": 182775540, "end": 182775699}, {"filename": "/GameData/shaders/compiled/web/gles/fs_ui_flatcolor.bin", "start": 182775699, "end": 182775881}, {"filename": "/GameData/shaders/compiled/web/gles/fs_ui_textured.bin", "start": 182775881, "end": 182776310}, {"filename": "/GameData/shaders/compiled/web/gles/fs_unlit.bin", "start": 182776310, "end": 182776811}, {"filename": "/GameData/shaders/compiled/web/gles/fs_unlit_rect.bin", "start": 182776811, "end": 182777398}, {"filename": "/GameData/shaders/compiled/web/gles/game/fs_debuff.bin", "start": 182777398, "end": 182779979}, {"filename": "/GameData/shaders/compiled/web/gles/game/fs_inventory_menu_sphere.bin", "start": 182779979, "end": 182780209}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_blendmask.bin", "start": 182780209, "end": 182780630}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_blur.bin", "start": 182780630, "end": 182783839}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_color.bin", "start": 182783839, "end": 182784000}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_colormatrix.bin", "start": 182784000, "end": 182784728}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_creation.bin", "start": 182784728, "end": 182784887}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_dropshadow.bin", "start": 182784887, "end": 182785317}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_gradient.bin", "start": 182785317, "end": 182789458}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_passthrough.bin", "start": 182789458, "end": 182789743}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/fs_rmlui_texture.bin", "start": 182789743, "end": 182790059}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/vs_rmlui.bin", "start": 182790059, "end": 182790555}, {"filename": "/GameData/shaders/compiled/web/gles/rmlui/vs_rmlui_passthrough.bin", "start": 182790555, "end": 182791073}, {"filename": "/GameData/shaders/compiled/web/gles/ui/fs_progressBar.bin", "start": 182791073, "end": 182791763}, {"filename": "/GameData/shaders/compiled/web/gles/vs_bsp.bin", "start": 182791763, "end": 182794258}, {"filename": "/GameData/shaders/compiled/web/gles/vs_default.bin", "start": 182794258, "end": 182799238}, {"filename": "/GameData/shaders/compiled/web/gles/vs_fullscreen.bin", "start": 182799238, "end": 182799541}, {"filename": "/GameData/shaders/compiled/web/gles/vs_instanced_billboard.bin", "start": 182799541, "end": 182800919}, {"filename": "/GameData/shaders/compiled/web/gles/vs_ui.bin", "start": 182800919, "end": 182801353}, {"filename": "/GameData/shaders/compiled/web/spirv/bsp/fs_bsp.bin", "start": 182801353, "end": 182810232}, {"filename": "/GameData/shaders/compiled/web/spirv/bsp/fs_bsp_cube.bin", "start": 182810232, "end": 182811809}, {"filename": "/GameData/shaders/compiled/web/spirv/bsp/fs_bsp_empty.bin", "start": 182811809, "end": 182812171}, {"filename": "/GameData/shaders/compiled/web/spirv/bsp/vs_bsp.bin", "start": 182812171, "end": 182815885}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_bsp.bin", "start": 182815885, "end": 182824764}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_bsp_cube.bin", "start": 182824764, "end": 182826341}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_bsp_empty.bin", "start": 182826341, "end": 182826703}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_copy.bin", "start": 182826703, "end": 182827549}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_copy_depth.bin", "start": 182827549, "end": 182828418}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_customId.bin", "start": 182828418, "end": 182830283}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_default.bin", "start": 182830283, "end": 182839450}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_default_simple.bin", "start": 182839450, "end": 182845543}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_empty.bin", "start": 182845543, "end": 182845905}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_fxaa_simple.bin", "start": 182845905, "end": 182849572}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_mask.bin", "start": 182849572, "end": 182850458}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_motionBlur.bin", "start": 182850458, "end": 182854676}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_motionBlur_apply.bin", "start": 182854676, "end": 182856036}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_postprocessing.bin", "start": 182856036, "end": 182865088}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_resolve_depth_msaa.bin", "start": 182865088, "end": 182866615}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_solidRed.bin", "start": 182866615, "end": 182866993}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_ui_flatcolor.bin", "start": 182866993, "end": 182867577}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_ui_textured.bin", "start": 182867577, "end": 182868793}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_unlit.bin", "start": 182868793, "end": 182870060}, {"filename": "/GameData/shaders/compiled/web/spirv/fs_unlit_rect.bin", "start": 182870060, "end": 182871537}, {"filename": "/GameData/shaders/compiled/web/spirv/game/fs_debuff.bin", "start": 182871537, "end": 182874502}, {"filename": "/GameData/shaders/compiled/web/spirv/game/fs_inventory_menu_sphere.bin", "start": 182874502, "end": 182874896}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_blendmask.bin", "start": 182874896, "end": 182875936}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_blur.bin", "start": 182875936, "end": 182878564}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_color.bin", "start": 182878564, "end": 182878970}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_colormatrix.bin", "start": 182878970, "end": 182880940}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_creation.bin", "start": 182880940, "end": 182881302}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_dropshadow.bin", "start": 182881302, "end": 182882553}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_gradient.bin", "start": 182882553, "end": 182885781}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_passthrough.bin", "start": 182885781, "end": 182886536}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/fs_rmlui_texture.bin", "start": 182886536, "end": 182887399}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/vs_rmlui.bin", "start": 182887399, "end": 182888713}, {"filename": "/GameData/shaders/compiled/web/spirv/rmlui/vs_rmlui_passthrough.bin", "start": 182888713, "end": 182890115}, {"filename": "/GameData/shaders/compiled/web/spirv/ui/fs_progressBar.bin", "start": 182890115, "end": 182892024}, {"filename": "/GameData/shaders/compiled/web/spirv/vs_bsp.bin", "start": 182892024, "end": 182895738}, {"filename": "/GameData/shaders/compiled/web/spirv/vs_default.bin", "start": 182895738, "end": 182902449}, {"filename": "/GameData/shaders/compiled/web/spirv/vs_fullscreen.bin", "start": 182902449, "end": 182903187}, {"filename": "/GameData/shaders/compiled/web/spirv/vs_instanced_billboard.bin", "start": 182903187, "end": 182906023}, {"filename": "/GameData/shaders/compiled/web/spirv/vs_ui.bin", "start": 182906023, "end": 182907349}, {"filename": "/GameData/shaders/compiled/windows/dx11/bsp/fs_bsp.bin", "start": 182907349, "end": 182913108}, {"filename": "/GameData/shaders/compiled/windows/dx11/bsp/fs_bsp_cube.bin", "start": 182913108, "end": 182914141}, {"filename": "/GameData/shaders/compiled/windows/dx11/bsp/fs_bsp_empty.bin", "start": 182914141, "end": 182914587}, {"filename": "/GameData/shaders/compiled/windows/dx11/bsp/vs_bsp.bin", "start": 182914587, "end": 182918549}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_bsp.bin", "start": 182918549, "end": 182924308}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_bsp_cube.bin", "start": 182924308, "end": 182925341}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_bsp_empty.bin", "start": 182925341, "end": 182925787}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_copy.bin", "start": 182925787, "end": 182926277}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_copy_depth.bin", "start": 182926277, "end": 182926778}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_customId.bin", "start": 182926778, "end": 182928015}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_default.bin", "start": 182928015, "end": 182940714}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_default_simple.bin", "start": 182940714, "end": 182945051}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_empty.bin", "start": 182945051, "end": 182945473}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_fxaa_simple.bin", "start": 182945473, "end": 182949200}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_mask.bin", "start": 182949200, "end": 182949774}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_motionBlur.bin", "start": 182949774, "end": 182953512}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_motionBlur_apply.bin", "start": 182953512, "end": 182954424}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_postprocessing.bin", "start": 182954424, "end": 182963020}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_resolve_depth_msaa.bin", "start": 182963020, "end": 182963995}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_solidRed.bin", "start": 182963995, "end": 182964445}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_ui_flatcolor.bin", "start": 182964445, "end": 182964745}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_ui_textured.bin", "start": 182964745, "end": 182965233}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_unlit.bin", "start": 182965233, "end": 182965980}, {"filename": "/GameData/shaders/compiled/windows/dx11/fs_unlit_rect.bin", "start": 182965980, "end": 182966965}, {"filename": "/GameData/shaders/compiled/windows/dx11/game/fs_debuff.bin", "start": 182966965, "end": 182970806}, {"filename": "/GameData/shaders/compiled/windows/dx11/game/fs_inventory_menu_sphere.bin", "start": 182970806, "end": 182971296}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_blendmask.bin", "start": 182971296, "end": 182971884}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_blur.bin", "start": 182971884, "end": 182973800}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_color.bin", "start": 182973800, "end": 182974126}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_colormatrix.bin", "start": 182974126, "end": 182975084}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_creation.bin", "start": 182975084, "end": 182975410}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_dropshadow.bin", "start": 182975410, "end": 182976049}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_gradient.bin", "start": 182976049, "end": 182980057}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_passthrough.bin", "start": 182980057, "end": 182980484}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/fs_rmlui_texture.bin", "start": 182980484, "end": 182980951}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/vs_rmlui.bin", "start": 182980951, "end": 182981829}, {"filename": "/GameData/shaders/compiled/windows/dx11/rmlui/vs_rmlui_passthrough.bin", "start": 182981829, "end": 182982619}, {"filename": "/GameData/shaders/compiled/windows/dx11/ui/fs_progressBar.bin", "start": 182982619, "end": 182983340}, {"filename": "/GameData/shaders/compiled/windows/dx11/vs_bsp.bin", "start": 182983340, "end": 182987302}, {"filename": "/GameData/shaders/compiled/windows/dx11/vs_default.bin", "start": 182987302, "end": 182997613}, {"filename": "/GameData/shaders/compiled/windows/dx11/vs_fullscreen.bin", "start": 182997613, "end": 182998051}, {"filename": "/GameData/shaders/compiled/windows/dx11/vs_instanced_billboard.bin", "start": 182998051, "end": 183000511}, {"filename": "/GameData/shaders/compiled/windows/dx11/vs_ui.bin", "start": 183000511, "end": 183001402}, {"filename": "/GameData/shaders/compiled/windows/dx12/bsp/fs_bsp.bin", "start": 183001402, "end": 183007161}, {"filename": "/GameData/shaders/compiled/windows/dx12/bsp/fs_bsp_cube.bin", "start": 183007161, "end": 183008194}, {"filename": "/GameData/shaders/compiled/windows/dx12/bsp/fs_bsp_empty.bin", "start": 183008194, "end": 183008640}, {"filename": "/GameData/shaders/compiled/windows/dx12/bsp/vs_bsp.bin", "start": 183008640, "end": 183012602}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_bsp.bin", "start": 183012602, "end": 183018361}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_bsp_cube.bin", "start": 183018361, "end": 183019394}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_bsp_empty.bin", "start": 183019394, "end": 183019840}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_copy.bin", "start": 183019840, "end": 183020330}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_copy_depth.bin", "start": 183020330, "end": 183020831}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_customId.bin", "start": 183020831, "end": 183022068}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_default.bin", "start": 183022068, "end": 183034767}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_default_simple.bin", "start": 183034767, "end": 183039104}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_empty.bin", "start": 183039104, "end": 183039526}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_fxaa_simple.bin", "start": 183039526, "end": 183043253}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_mask.bin", "start": 183043253, "end": 183043827}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_motionBlur.bin", "start": 183043827, "end": 183047565}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_motionBlur_apply.bin", "start": 183047565, "end": 183048477}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_postprocessing.bin", "start": 183048477, "end": 183057073}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_resolve_depth_msaa.bin", "start": 183057073, "end": 183058048}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_solidRed.bin", "start": 183058048, "end": 183058498}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_ui_flatcolor.bin", "start": 183058498, "end": 183058798}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_ui_textured.bin", "start": 183058798, "end": 183059286}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_unlit.bin", "start": 183059286, "end": 183060033}, {"filename": "/GameData/shaders/compiled/windows/dx12/fs_unlit_rect.bin", "start": 183060033, "end": 183061018}, {"filename": "/GameData/shaders/compiled/windows/dx12/game/fs_debuff.bin", "start": 183061018, "end": 183064859}, {"filename": "/GameData/shaders/compiled/windows/dx12/game/fs_inventory_menu_sphere.bin", "start": 183064859, "end": 183065349}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_blendmask.bin", "start": 183065349, "end": 183065937}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_blur.bin", "start": 183065937, "end": 183067853}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_color.bin", "start": 183067853, "end": 183068179}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_colormatrix.bin", "start": 183068179, "end": 183069137}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_creation.bin", "start": 183069137, "end": 183069463}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_dropshadow.bin", "start": 183069463, "end": 183070102}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_gradient.bin", "start": 183070102, "end": 183074110}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_passthrough.bin", "start": 183074110, "end": 183074537}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/fs_rmlui_texture.bin", "start": 183074537, "end": 183075004}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/vs_rmlui.bin", "start": 183075004, "end": 183075882}, {"filename": "/GameData/shaders/compiled/windows/dx12/rmlui/vs_rmlui_passthrough.bin", "start": 183075882, "end": 183076672}, {"filename": "/GameData/shaders/compiled/windows/dx12/ui/fs_progressBar.bin", "start": 183076672, "end": 183077393}, {"filename": "/GameData/shaders/compiled/windows/dx12/vs_bsp.bin", "start": 183077393, "end": 183081355}, {"filename": "/GameData/shaders/compiled/windows/dx12/vs_default.bin", "start": 183081355, "end": 183091666}, {"filename": "/GameData/shaders/compiled/windows/dx12/vs_fullscreen.bin", "start": 183091666, "end": 183092104}, {"filename": "/GameData/shaders/compiled/windows/dx12/vs_instanced_billboard.bin", "start": 183092104, "end": 183094564}, {"filename": "/GameData/shaders/compiled/windows/dx12/vs_ui.bin", "start": 183094564, "end": 183095455}, {"filename": "/GameData/shaders/compiled/windows/gl/bsp/fs_bsp.bin", "start": 183095455, "end": 183100906}, {"filename": "/GameData/shaders/compiled/windows/gl/bsp/fs_bsp_cube.bin", "start": 183100906, "end": 183101331}, {"filename": "/GameData/shaders/compiled/windows/gl/bsp/fs_bsp_empty.bin", "start": 183101331, "end": 183101411}, {"filename": "/GameData/shaders/compiled/windows/gl/bsp/vs_bsp.bin", "start": 183101411, "end": 183103706}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_bsp.bin", "start": 183103706, "end": 183109157}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_bsp_cube.bin", "start": 183109157, "end": 183109582}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_bsp_empty.bin", "start": 183109582, "end": 183109662}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_copy.bin", "start": 183109662, "end": 183109832}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_copy_depth.bin", "start": 183109832, "end": 183110001}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_customId.bin", "start": 183110001, "end": 183110784}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_default.bin", "start": 183110784, "end": 183118533}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_default_simple.bin", "start": 183118533, "end": 183122328}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_empty.bin", "start": 183122328, "end": 183122408}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_fxaa_simple.bin", "start": 183122408, "end": 183125866}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_mask.bin", "start": 183125866, "end": 183126127}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_motionBlur.bin", "start": 183126127, "end": 183128151}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_motionBlur_apply.bin", "start": 183128151, "end": 183128563}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_postprocessing.bin", "start": 183128563, "end": 183135380}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_resolve_depth_msaa.bin", "start": 183135380, "end": 183135956}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_solidRed.bin", "start": 183135956, "end": 183136036}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_ui_flatcolor.bin", "start": 183136036, "end": 183136139}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_ui_textured.bin", "start": 183136139, "end": 183136479}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_unlit.bin", "start": 183136479, "end": 183136891}, {"filename": "/GameData/shaders/compiled/windows/gl/fs_unlit_rect.bin", "start": 183136891, "end": 183137389}, {"filename": "/GameData/shaders/compiled/windows/gl/game/fs_debuff.bin", "start": 183137389, "end": 183139866}, {"filename": "/GameData/shaders/compiled/windows/gl/game/fs_inventory_menu_sphere.bin", "start": 183139866, "end": 183140009}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_blendmask.bin", "start": 183140009, "end": 183140261}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_blur.bin", "start": 183140261, "end": 183143386}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_color.bin", "start": 183143386, "end": 183143468}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_colormatrix.bin", "start": 183143468, "end": 183144097}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_creation.bin", "start": 183144097, "end": 183144177}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_dropshadow.bin", "start": 183144177, "end": 183144483}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_gradient.bin", "start": 183144483, "end": 183148529}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_passthrough.bin", "start": 183148529, "end": 183148690}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/fs_rmlui_texture.bin", "start": 183148690, "end": 183148882}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/vs_rmlui.bin", "start": 183148882, "end": 183149286}, {"filename": "/GameData/shaders/compiled/windows/gl/rmlui/vs_rmlui_passthrough.bin", "start": 183149286, "end": 183149712}, {"filename": "/GameData/shaders/compiled/windows/gl/ui/fs_progressBar.bin", "start": 183149712, "end": 183150313}, {"filename": "/GameData/shaders/compiled/windows/gl/vs_bsp.bin", "start": 183150313, "end": 183152608}, {"filename": "/GameData/shaders/compiled/windows/gl/vs_default.bin", "start": 183152608, "end": 183157274}, {"filename": "/GameData/shaders/compiled/windows/gl/vs_fullscreen.bin", "start": 183157274, "end": 183157509}, {"filename": "/GameData/shaders/compiled/windows/gl/vs_instanced_billboard.bin", "start": 183157509, "end": 183158711}, {"filename": "/GameData/shaders/compiled/windows/gl/vs_ui.bin", "start": 183158711, "end": 183159065}, {"filename": "/GameData/shaders/compiled/windows/spirv/bsp/fs_bsp.bin", "start": 183159065, "end": 183167944}, {"filename": "/GameData/shaders/compiled/windows/spirv/bsp/fs_bsp_cube.bin", "start": 183167944, "end": 183169521}, {"filename": "/GameData/shaders/compiled/windows/spirv/bsp/fs_bsp_empty.bin", "start": 183169521, "end": 183169883}, {"filename": "/GameData/shaders/compiled/windows/spirv/bsp/vs_bsp.bin", "start": 183169883, "end": 183173597}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_bsp.bin", "start": 183173597, "end": 183182476}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_bsp_cube.bin", "start": 183182476, "end": 183184053}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_bsp_empty.bin", "start": 183184053, "end": 183184415}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_copy.bin", "start": 183184415, "end": 183185261}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_copy_depth.bin", "start": 183185261, "end": 183186130}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_customId.bin", "start": 183186130, "end": 183187995}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_default.bin", "start": 183187995, "end": 183197162}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_default_simple.bin", "start": 183197162, "end": 183203255}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_empty.bin", "start": 183203255, "end": 183203617}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_fxaa_simple.bin", "start": 183203617, "end": 183207284}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_mask.bin", "start": 183207284, "end": 183208170}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_motionBlur.bin", "start": 183208170, "end": 183212388}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_motionBlur_apply.bin", "start": 183212388, "end": 183213748}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_postprocessing.bin", "start": 183213748, "end": 183222800}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_resolve_depth_msaa.bin", "start": 183222800, "end": 183224327}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_solidRed.bin", "start": 183224327, "end": 183224705}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_ui_flatcolor.bin", "start": 183224705, "end": 183225289}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_ui_textured.bin", "start": 183225289, "end": 183226505}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_unlit.bin", "start": 183226505, "end": 183227772}, {"filename": "/GameData/shaders/compiled/windows/spirv/fs_unlit_rect.bin", "start": 183227772, "end": 183229249}, {"filename": "/GameData/shaders/compiled/windows/spirv/game/fs_debuff.bin", "start": 183229249, "end": 183232214}, {"filename": "/GameData/shaders/compiled/windows/spirv/game/fs_inventory_menu_sphere.bin", "start": 183232214, "end": 183232608}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_blendmask.bin", "start": 183232608, "end": 183233648}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_blur.bin", "start": 183233648, "end": 183236276}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_color.bin", "start": 183236276, "end": 183236682}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_colormatrix.bin", "start": 183236682, "end": 183238652}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_creation.bin", "start": 183238652, "end": 183239014}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_dropshadow.bin", "start": 183239014, "end": 183240265}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_gradient.bin", "start": 183240265, "end": 183243493}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_passthrough.bin", "start": 183243493, "end": 183244248}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/fs_rmlui_texture.bin", "start": 183244248, "end": 183245111}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/vs_rmlui.bin", "start": 183245111, "end": 183246425}, {"filename": "/GameData/shaders/compiled/windows/spirv/rmlui/vs_rmlui_passthrough.bin", "start": 183246425, "end": 183247827}, {"filename": "/GameData/shaders/compiled/windows/spirv/ui/fs_progressBar.bin", "start": 183247827, "end": 183249736}, {"filename": "/GameData/shaders/compiled/windows/spirv/vs_bsp.bin", "start": 183249736, "end": 183253450}, {"filename": "/GameData/shaders/compiled/windows/spirv/vs_default.bin", "start": 183253450, "end": 183260161}, {"filename": "/GameData/shaders/compiled/windows/spirv/vs_fullscreen.bin", "start": 183260161, "end": 183260899}, {"filename": "/GameData/shaders/compiled/windows/spirv/vs_instanced_billboard.bin", "start": 183260899, "end": 183263735}, {"filename": "/GameData/shaders/compiled/windows/spirv/vs_ui.bin", "start": 183263735, "end": 183265061}, {"filename": "/GameData/shaders/shaderc.exe", "start": 183265061, "end": 224840485}, {"filename": "/GameData/shaders/source/bgfx_compute.sh", "start": 224840485, "end": 224858906}, {"filename": "/GameData/shaders/source/bgfx_shader.sh", "start": 224858906, "end": 224886865}, {"filename": "/GameData/shaders/source/bsp/bgfx_shader.sh", "start": 224886865, "end": 224914824}, {"filename": "/GameData/shaders/source/bsp/fs_bsp.sc", "start": 224914824, "end": 224918972}, {"filename": "/GameData/shaders/source/bsp/fs_bsp_cube.sc", "start": 224918972, "end": 224919414}, {"filename": "/GameData/shaders/source/bsp/fs_bsp_empty.sc", "start": 224919414, "end": 224919631}, {"filename": "/GameData/shaders/source/bsp/varying.def.sc", "start": 224919631, "end": 224920599}, {"filename": "/GameData/shaders/source/bsp/vs_bsp.sc", "start": 224920599, "end": 224921873}, {"filename": "/GameData/shaders/source/fs_copy.sc", "start": 224921873, "end": 224922127}, {"filename": "/GameData/shaders/source/fs_copy_depth.sc", "start": 224922127, "end": 224922377}, {"filename": "/GameData/shaders/source/fs_customId.sc", "start": 224922377, "end": 224923084}, {"filename": "/GameData/shaders/source/fs_default.sc", "start": 224923084, "end": 224931837}, {"filename": "/GameData/shaders/source/fs_default_simple.sc", "start": 224931837, "end": 224935408}, {"filename": "/GameData/shaders/source/fs_empty.sc", "start": 224935408, "end": 224935610}, {"filename": "/GameData/shaders/source/fs_fxaa_simple.sc", "start": 224935610, "end": 224939800}, {"filename": "/GameData/shaders/source/fs_mask.sc", "start": 224939800, "end": 224940124}, {"filename": "/GameData/shaders/source/fs_motionBlur.sc", "start": 224940124, "end": 224942669}, {"filename": "/GameData/shaders/source/fs_motionBlur_apply.sc", "start": 224942669, "end": 224943262}, {"filename": "/GameData/shaders/source/fs_postprocessing.sc", "start": 224943262, "end": 224950531}, {"filename": "/GameData/shaders/source/fs_resolve_depth_msaa.sc", "start": 224950531, "end": 224951428}, {"filename": "/GameData/shaders/source/fs_solidRed.sc", "start": 224951428, "end": 224951648}, {"filename": "/GameData/shaders/source/fs_ui_flatcolor.sc", "start": 224951648, "end": 224951772}, {"filename": "/GameData/shaders/source/fs_ui_textured.sc", "start": 224951772, "end": 224952058}, {"filename": "/GameData/shaders/source/fs_unlit.sc", "start": 224952058, "end": 224952455}, {"filename": "/GameData/shaders/source/fs_unlit_rect.sc", "start": 224952455, "end": 224953064}, {"filename": "/GameData/shaders/source/game/fs_debuff.sc", "start": 224953064, "end": 224955893}, {"filename": "/GameData/shaders/source/game/fs_inventory_menu_sphere.sc", "start": 224955893, "end": 224956504}, {"filename": "/GameData/shaders/source/gl/bsp.frag", "start": 224956504, "end": 224959907}, {"filename": "/GameData/shaders/source/gl/bsp.vert", "start": 224959907, "end": 224960740}, {"filename": "/GameData/shaders/source/gl/bsp_cube.frag", "start": 224960740, "end": 224961167}, {"filename": "/GameData/shaders/source/gl/customId.frag", "start": 224961167, "end": 224962069}, {"filename": "/GameData/shaders/source/gl/default_pixel.frag", "start": 224962069, "end": 224965607}, {"filename": "/GameData/shaders/source/gl/default_pixel_.frag", "start": 224965607, "end": 224972566}, {"filename": "/GameData/shaders/source/gl/default_pixel_shaded.frag", "start": 224972566, "end": 224983301}, {"filename": "/GameData/shaders/source/gl/default_pixel_simple.frag", "start": 224983301, "end": 224994132}, {"filename": "/GameData/shaders/source/gl/default_vertex.vert", "start": 224994132, "end": 224996679}, {"filename": "/GameData/shaders/source/gl/empty_pixel.frag", "start": 224996679, "end": 224996746}, {"filename": "/GameData/shaders/source/gl/fog_particle.frag", "start": 224996746, "end": 224998265}, {"filename": "/GameData/shaders/source/gl/fullscreen_vertex.vert", "start": 224998265, "end": 224998469}, {"filename": "/GameData/shaders/source/gl/fxaa_simple.frag", "start": 224998469, "end": 225003872}, {"filename": "/GameData/shaders/source/gl/game/debuff.frag", "start": 225003872, "end": 225006351}, {"filename": "/GameData/shaders/source/gl/game/inventory_menu_sphere.frag", "start": 225006351, "end": 225006929}, {"filename": "/GameData/shaders/source/gl/instanced_bilboard_vertex.vert", "start": 225006929, "end": 225008868}, {"filename": "/GameData/shaders/source/gl/mask_pixel.frag", "start": 225008868, "end": 225009308}, {"filename": "/GameData/shaders/source/gl/motionBlur.frag", "start": 225009308, "end": 225012476}, {"filename": "/GameData/shaders/source/gl/motionBlur_apply.frag", "start": 225012476, "end": 225012863}, {"filename": "/GameData/shaders/source/gl/postprocessing.frag", "start": 225012863, "end": 225021872}, {"filename": "/GameData/shaders/source/gl/solidRed_pixel.frag", "start": 225021872, "end": 225022041}, {"filename": "/GameData/shaders/source/gl/texture_pixel.frag", "start": 225022041, "end": 225022227}, {"filename": "/GameData/shaders/source/gl/ui.vert", "start": 225022227, "end": 225022999}, {"filename": "/GameData/shaders/source/gl/ui/progressBar.frag", "start": 225022999, "end": 225023789}, {"filename": "/GameData/shaders/source/gl/ui_flatcolor.frag", "start": 225023789, "end": 225023923}, {"filename": "/GameData/shaders/source/gl/ui_sliced.frag", "start": 225023923, "end": 225030813}, {"filename": "/GameData/shaders/source/gl/ui_textured.frag", "start": 225030813, "end": 225031134}, {"filename": "/GameData/shaders/source/gl/unlit_pixel.frag", "start": 225031134, "end": 225031576}, {"filename": "/GameData/shaders/source/rmlui/bgfx_shader.sh", "start": 225031576, "end": 225059535}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_blendmask.sc", "start": 225059535, "end": 225060165}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_blur.sc", "start": 225060165, "end": 225061803}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_color.sc", "start": 225061803, "end": 225062094}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_colormatrix.sc", "start": 225062094, "end": 225063166}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_creation.sc", "start": 225063166, "end": 225063795}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_dropshadow.sc", "start": 225063795, "end": 225064686}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_gradient.sc", "start": 225064686, "end": 225067664}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_passthrough.sc", "start": 225067664, "end": 225068033}, {"filename": "/GameData/shaders/source/rmlui/fs_rmlui_texture.sc", "start": 225068033, "end": 225068461}, {"filename": "/GameData/shaders/source/rmlui/varying.def.sc", "start": 225068461, "end": 225068705}, {"filename": "/GameData/shaders/source/rmlui/vs_rmlui.sc", "start": 225068705, "end": 225069516}, {"filename": "/GameData/shaders/source/rmlui/vs_rmlui_passthrough.sc", "start": 225069516, "end": 225070378}, {"filename": "/GameData/shaders/source/ui/fs_progressBar.sc", "start": 225070378, "end": 225071112}, {"filename": "/GameData/shaders/source/varying.def.sc", "start": 225071112, "end": 225072088}, {"filename": "/GameData/shaders/source/vs_default.sc", "start": 225072088, "end": 225075586}, {"filename": "/GameData/shaders/source/vs_fullscreen.sc", "start": 225075586, "end": 225075771}, {"filename": "/GameData/shaders/source/vs_instanced_billboard.sc", "start": 225075771, "end": 225076735}, {"filename": "/GameData/shaders/source/vs_ui.sc", "start": 225076735, "end": 225077184}, {"filename": "/GameData/sounds/banks/Desktop/Dialogue_CN.bank", "start": 225077184, "end": 225129600}, {"filename": "/GameData/sounds/banks/Desktop/Dialogue_EN.bank", "start": 225129600, "end": 225190720}, {"filename": "/GameData/sounds/banks/Desktop/Dialogue_JP.bank", "start": 225190720, "end": 225257280}, {"filename": "/GameData/sounds/banks/Desktop/Master.bank", "start": 225257280, "end": 225457472}, {"filename": "/GameData/sounds/banks/Desktop/Master.strings.bank", "start": 225457472, "end": 225462152}, {"filename": "/GameData/sounds/banks/Desktop/Music.bank", "start": 225462152, "end": 227999976}, {"filename": "/GameData/sounds/banks/Desktop/SFX.bank", "start": 227999976, "end": 250576008}, {"filename": "/GameData/sounds/banks/Desktop/VO.bank", "start": 250576008, "end": 250840264}, {"filename": "/GameData/sounds/banks/Desktop/Weapons.bank", "start": 250840264, "end": 250955752}, {"filename": "/GameData/sounds/dog/death.wav", "start": 250955752, "end": 251029138, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_attack.wav", "start": 251029138, "end": 251141662, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_attack_start.wav", "start": 251141662, "end": 251338346, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_death.wav", "start": 251338346, "end": 251492546, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_hit.wav", "start": 251492546, "end": 251566238, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_stun.wav", "start": 251566238, "end": 251659688, "audio": 1}, {"filename": "/GameData/sounds/mew.wav", "start": 251659688, "end": 251733074, "audio": 1}, {"filename": "/GameData/sounds/weapons/shotgun/shotgun_fire.wav", "start": 251733074, "end": 252193958, "audio": 1}, {"filename": "/GameData/sounds/weapons/shotgun/shotgun_fire2.wav", "start": 252193958, "end": 252654842, "audio": 1}, {"filename": "/GameData/tables/items/items.csv", "start": 252654842, "end": 252656203}, {"filename": "/GameData/tables/npc/npc_example.csv", "start": 252656203, "end": 252656891}, {"filename": "/GameData/testViewmodel.glb", "start": 252656891, "end": 253528407}, {"filename": "/GameData/textures/FPSTextures/BigWall_01.png", "start": 253528407, "end": 253539834}, {"filename": "/GameData/textures/FPSTextures/BigWall_02.png", "start": 253539834, "end": 253547104}, {"filename": "/GameData/textures/FPSTextures/BigWall_03.png", "start": 253547104, "end": 253556772}, {"filename": "/GameData/textures/FPSTextures/BigWall_04.png", "start": 253556772, "end": 253571519}, {"filename": "/GameData/textures/FPSTextures/BigWall_05.png", "start": 253571519, "end": 253576902}, {"filename": "/GameData/textures/FPSTextures/Column01.png", "start": 253576902, "end": 253580572}, {"filename": "/GameData/textures/FPSTextures/Column02.png", "start": 253580572, "end": 253582713}, {"filename": "/GameData/textures/FPSTextures/Column03.png", "start": 253582713, "end": 253586115}, {"filename": "/GameData/textures/FPSTextures/Column04.png", "start": 253586115, "end": 253592313}, {"filename": "/GameData/textures/FPSTextures/Column05.png", "start": 253592313, "end": 253594797}, {"filename": "/GameData/textures/FPSTextures/Column06.png", "start": 253594797, "end": 253600581}, {"filename": "/GameData/textures/FPSTextures/Column07.png", "start": 253600581, "end": 253606557}, {"filename": "/GameData/textures/FPSTextures/Column08.png", "start": 253606557, "end": 253612728}, {"filename": "/GameData/textures/FPSTextures/Column09.png", "start": 253612728, "end": 253619256}, {"filename": "/GameData/textures/FPSTextures/Column10.png", "start": 253619256, "end": 253621028}, {"filename": "/GameData/textures/FPSTextures/Column11.png", "start": 253621028, "end": 253625700}, {"filename": "/GameData/textures/FPSTextures/Column12.png", "start": 253625700, "end": 253631516}, {"filename": "/GameData/textures/FPSTextures/Column_13.png", "start": 253631516, "end": 253633667}, {"filename": "/GameData/textures/FPSTextures/Column_14.png", "start": 253633667, "end": 253635434}, {"filename": "/GameData/textures/FPSTextures/Column_15.png", "start": 253635434, "end": 253637401}, {"filename": "/GameData/textures/FPSTextures/Column_16.png", "start": 253637401, "end": 253640082}, {"filename": "/GameData/textures/FPSTextures/Column_17.png", "start": 253640082, "end": 253642710}, {"filename": "/GameData/textures/FPSTextures/Column_18.png", "start": 253642710, "end": 253645524}, {"filename": "/GameData/textures/FPSTextures/Crate01.png", "start": 253645524, "end": 253651730}, {"filename": "/GameData/textures/FPSTextures/Crate02.png", "start": 253651730, "end": 253657798}, {"filename": "/GameData/textures/FPSTextures/Crate03.png", "start": 253657798, "end": 253660308}, {"filename": "/GameData/textures/FPSTextures/Door_01.png", "start": 253660308, "end": 253667383}, {"filename": "/GameData/textures/FPSTextures/Door_02.png", "start": 253667383, "end": 253677174}, {"filename": "/GameData/textures/FPSTextures/Door_03.png", "start": 253677174, "end": 253689350}, {"filename": "/GameData/textures/FPSTextures/Door_04.png", "start": 253689350, "end": 253697568}, {"filename": "/GameData/textures/FPSTextures/Flat01.png", "start": 253697568, "end": 253700231}, {"filename": "/GameData/textures/FPSTextures/Flat02.png", "start": 253700231, "end": 253702985}, {"filename": "/GameData/textures/FPSTextures/Flat03.png", "start": 253702985, "end": 253707560}, {"filename": "/GameData/textures/FPSTextures/Flat04.png", "start": 253707560, "end": 253711816}, {"filename": "/GameData/textures/FPSTextures/Flat05.png", "start": 253711816, "end": 253714258}, {"filename": "/GameData/textures/FPSTextures/Flat06.png", "start": 253714258, "end": 253716343}, {"filename": "/GameData/textures/FPSTextures/Flat07.png", "start": 253716343, "end": 253721765}, {"filename": "/GameData/textures/FPSTextures/Flat08.png", "start": 253721765, "end": 253727614}, {"filename": "/GameData/textures/FPSTextures/Flat09.png", "start": 253727614, "end": 253732233}, {"filename": "/GameData/textures/FPSTextures/Flat10.png", "start": 253732233, "end": 253737179}, {"filename": "/GameData/textures/FPSTextures/Flat100.png", "start": 253737179, "end": 253743091}, {"filename": "/GameData/textures/FPSTextures/Flat101.png", "start": 253743091, "end": 253745878}, {"filename": "/GameData/textures/FPSTextures/Flat102.png", "start": 253745878, "end": 253748044}, {"filename": "/GameData/textures/FPSTextures/Flat103.png", "start": 253748044, "end": 253749591}, {"filename": "/GameData/textures/FPSTextures/Flat104.png", "start": 253749591, "end": 253751593}, {"filename": "/GameData/textures/FPSTextures/Flat105.png", "start": 253751593, "end": 253753264}, {"filename": "/GameData/textures/FPSTextures/Flat106.png", "start": 253753264, "end": 253755387}, {"filename": "/GameData/textures/FPSTextures/Flat107.png", "start": 253755387, "end": 253757765}, {"filename": "/GameData/textures/FPSTextures/Flat108.png", "start": 253757765, "end": 253759602}, {"filename": "/GameData/textures/FPSTextures/Flat109.png", "start": 253759602, "end": 253764294}, {"filename": "/GameData/textures/FPSTextures/Flat11.png", "start": 253764294, "end": 253766919}, {"filename": "/GameData/textures/FPSTextures/Flat110.png", "start": 253766919, "end": 253769121}, {"filename": "/GameData/textures/FPSTextures/Flat111.png", "start": 253769121, "end": 253774454}, {"filename": "/GameData/textures/FPSTextures/Flat112.png", "start": 253774454, "end": 253776345}, {"filename": "/GameData/textures/FPSTextures/Flat113.png", "start": 253776345, "end": 253778499}, {"filename": "/GameData/textures/FPSTextures/Flat114.png", "start": 253778499, "end": 253780382}, {"filename": "/GameData/textures/FPSTextures/Flat115.png", "start": 253780382, "end": 253783148}, {"filename": "/GameData/textures/FPSTextures/Flat116.png", "start": 253783148, "end": 253788723}, {"filename": "/GameData/textures/FPSTextures/Flat117.png", "start": 253788723, "end": 253794853}, {"filename": "/GameData/textures/FPSTextures/Flat118.png", "start": 253794853, "end": 253799945}, {"filename": "/GameData/textures/FPSTextures/Flat119.png", "start": 253799945, "end": 253802572}, {"filename": "/GameData/textures/FPSTextures/Flat12.png", "start": 253802572, "end": 253805154}, {"filename": "/GameData/textures/FPSTextures/Flat120.png", "start": 253805154, "end": 253809540}, {"filename": "/GameData/textures/FPSTextures/Flat121.png", "start": 253809540, "end": 253815068}, {"filename": "/GameData/textures/FPSTextures/Flat122.png", "start": 253815068, "end": 253820478}, {"filename": "/GameData/textures/FPSTextures/Flat123.png", "start": 253820478, "end": 253826274}, {"filename": "/GameData/textures/FPSTextures/Flat124.png", "start": 253826274, "end": 253827734}, {"filename": "/GameData/textures/FPSTextures/Flat125.png", "start": 253827734, "end": 253832291}, {"filename": "/GameData/textures/FPSTextures/Flat126.png", "start": 253832291, "end": 253838493}, {"filename": "/GameData/textures/FPSTextures/Flat127.png", "start": 253838493, "end": 253844543}, {"filename": "/GameData/textures/FPSTextures/Flat128.png", "start": 253844543, "end": 253846750}, {"filename": "/GameData/textures/FPSTextures/Flat129.png", "start": 253846750, "end": 253849597}, {"filename": "/GameData/textures/FPSTextures/Flat13.png", "start": 253849597, "end": 253855243}, {"filename": "/GameData/textures/FPSTextures/Flat130.png", "start": 253855243, "end": 253861333}, {"filename": "/GameData/textures/FPSTextures/Flat131.png", "start": 253861333, "end": 253863603}, {"filename": "/GameData/textures/FPSTextures/Flat132.png", "start": 253863603, "end": 253865171}, {"filename": "/GameData/textures/FPSTextures/Flat133.png", "start": 253865171, "end": 253868816}, {"filename": "/GameData/textures/FPSTextures/Flat134.png", "start": 253868816, "end": 253878377}, {"filename": "/GameData/textures/FPSTextures/Flat135.png", "start": 253878377, "end": 253884742}, {"filename": "/GameData/textures/FPSTextures/Flat136.png", "start": 253884742, "end": 253887348}, {"filename": "/GameData/textures/FPSTextures/Flat137.png", "start": 253887348, "end": 253889852}, {"filename": "/GameData/textures/FPSTextures/Flat138.png", "start": 253889852, "end": 253894729}, {"filename": "/GameData/textures/FPSTextures/Flat139.png", "start": 253894729, "end": 253896345}, {"filename": "/GameData/textures/FPSTextures/Flat14.png", "start": 253896345, "end": 253898117}, {"filename": "/GameData/textures/FPSTextures/Flat140.png", "start": 253898117, "end": 253905623}, {"filename": "/GameData/textures/FPSTextures/Flat141.png", "start": 253905623, "end": 253908816}, {"filename": "/GameData/textures/FPSTextures/Flat142.png", "start": 253908816, "end": 253914629}, {"filename": "/GameData/textures/FPSTextures/Flat15.png", "start": 253914629, "end": 253918378}, {"filename": "/GameData/textures/FPSTextures/Flat16.png", "start": 253918378, "end": 253922323}, {"filename": "/GameData/textures/FPSTextures/Flat17.png", "start": 253922323, "end": 253924374}, {"filename": "/GameData/textures/FPSTextures/Flat18.png", "start": 253924374, "end": 253929764}, {"filename": "/GameData/textures/FPSTextures/Flat19.png", "start": 253929764, "end": 253934624}, {"filename": "/GameData/textures/FPSTextures/Flat20.png", "start": 253934624, "end": 253940996}, {"filename": "/GameData/textures/FPSTextures/Flat21.png", "start": 253940996, "end": 253946020}, {"filename": "/GameData/textures/FPSTextures/Flat22.png", "start": 253946020, "end": 253949297}, {"filename": "/GameData/textures/FPSTextures/Flat23.png", "start": 253949297, "end": 253952168}, {"filename": "/GameData/textures/FPSTextures/Flat24.png", "start": 253952168, "end": 253954666}, {"filename": "/GameData/textures/FPSTextures/Flat25.png", "start": 253954666, "end": 253957345}, {"filename": "/GameData/textures/FPSTextures/Flat26.png", "start": 253957345, "end": 253961985}, {"filename": "/GameData/textures/FPSTextures/Flat27.png", "start": 253961985, "end": 253968532}, {"filename": "/GameData/textures/FPSTextures/Flat28.png", "start": 253968532, "end": 253974311}, {"filename": "/GameData/textures/FPSTextures/Flat29.png", "start": 253974311, "end": 253980049}, {"filename": "/GameData/textures/FPSTextures/Flat30.png", "start": 253980049, "end": 253982017}, {"filename": "/GameData/textures/FPSTextures/Flat31.png", "start": 253982017, "end": 253984518}, {"filename": "/GameData/textures/FPSTextures/Flat32.png", "start": 253984518, "end": 253990003}, {"filename": "/GameData/textures/FPSTextures/Flat33.png", "start": 253990003, "end": 253992378}, {"filename": "/GameData/textures/FPSTextures/Flat34.png", "start": 253992378, "end": 253994747}, {"filename": "/GameData/textures/FPSTextures/Flat35.png", "start": 253994747, "end": 253996840}, {"filename": "/GameData/textures/FPSTextures/Flat36.png", "start": 253996840, "end": 253998730}, {"filename": "/GameData/textures/FPSTextures/Flat37.png", "start": 253998730, "end": 254004597}, {"filename": "/GameData/textures/FPSTextures/Flat38.png", "start": 254004597, "end": 254010445}, {"filename": "/GameData/textures/FPSTextures/Flat39.png", "start": 254010445, "end": 254015866}, {"filename": "/GameData/textures/FPSTextures/Flat40.png", "start": 254015866, "end": 254021592}, {"filename": "/GameData/textures/FPSTextures/Flat41.png", "start": 254021592, "end": 254023358}, {"filename": "/GameData/textures/FPSTextures/Flat42.png", "start": 254023358, "end": 254029626}, {"filename": "/GameData/textures/FPSTextures/Flat43.png", "start": 254029626, "end": 254032280}, {"filename": "/GameData/textures/FPSTextures/Flat44.png", "start": 254032280, "end": 254038231}, {"filename": "/GameData/textures/FPSTextures/Flat45.png", "start": 254038231, "end": 254040876}, {"filename": "/GameData/textures/FPSTextures/Flat46.png", "start": 254040876, "end": 254042663}, {"filename": "/GameData/textures/FPSTextures/Flat47.png", "start": 254042663, "end": 254044572}, {"filename": "/GameData/textures/FPSTextures/Flat48.png", "start": 254044572, "end": 254046989}, {"filename": "/GameData/textures/FPSTextures/Flat49.png", "start": 254046989, "end": 254049891}, {"filename": "/GameData/textures/FPSTextures/Flat50.png", "start": 254049891, "end": 254055852}, {"filename": "/GameData/textures/FPSTextures/Flat51.png", "start": 254055852, "end": 254061379}, {"filename": "/GameData/textures/FPSTextures/Flat52.png", "start": 254061379, "end": 254063315}, {"filename": "/GameData/textures/FPSTextures/Flat53.png", "start": 254063315, "end": 254068842}, {"filename": "/GameData/textures/FPSTextures/Flat54.png", "start": 254068842, "end": 254075684}, {"filename": "/GameData/textures/FPSTextures/Flat55.png", "start": 254075684, "end": 254077845}, {"filename": "/GameData/textures/FPSTextures/Flat56.png", "start": 254077845, "end": 254084512}, {"filename": "/GameData/textures/FPSTextures/Flat57.png", "start": 254084512, "end": 254090160}, {"filename": "/GameData/textures/FPSTextures/Flat58.png", "start": 254090160, "end": 254092150}, {"filename": "/GameData/textures/FPSTextures/Flat59.png", "start": 254092150, "end": 254094079}, {"filename": "/GameData/textures/FPSTextures/Flat60.png", "start": 254094079, "end": 254095888}, {"filename": "/GameData/textures/FPSTextures/Flat61.png", "start": 254095888, "end": 254098458}, {"filename": "/GameData/textures/FPSTextures/Flat62.png", "start": 254098458, "end": 254104625}, {"filename": "/GameData/textures/FPSTextures/Flat63.png", "start": 254104625, "end": 254110646}, {"filename": "/GameData/textures/FPSTextures/Flat64.png", "start": 254110646, "end": 254116381}, {"filename": "/GameData/textures/FPSTextures/Flat65.png", "start": 254116381, "end": 254119269}, {"filename": "/GameData/textures/FPSTextures/Flat66.png", "start": 254119269, "end": 254121196}, {"filename": "/GameData/textures/FPSTextures/Flat67.png", "start": 254121196, "end": 254123229}, {"filename": "/GameData/textures/FPSTextures/Flat68.png", "start": 254123229, "end": 254125391}, {"filename": "/GameData/textures/FPSTextures/Flat69.png", "start": 254125391, "end": 254130885}, {"filename": "/GameData/textures/FPSTextures/Flat70.png", "start": 254130885, "end": 254136461}, {"filename": "/GameData/textures/FPSTextures/Flat71.png", "start": 254136461, "end": 254138325}, {"filename": "/GameData/textures/FPSTextures/Flat72.png", "start": 254138325, "end": 254141370}, {"filename": "/GameData/textures/FPSTextures/Flat73.png", "start": 254141370, "end": 254144093}, {"filename": "/GameData/textures/FPSTextures/Flat74.png", "start": 254144093, "end": 254146726}, {"filename": "/GameData/textures/FPSTextures/Flat75.png", "start": 254146726, "end": 254149602}, {"filename": "/GameData/textures/FPSTextures/Flat76.png", "start": 254149602, "end": 254151647}, {"filename": "/GameData/textures/FPSTextures/Flat77.png", "start": 254151647, "end": 254153837}, {"filename": "/GameData/textures/FPSTextures/Flat78.png", "start": 254153837, "end": 254157186}, {"filename": "/GameData/textures/FPSTextures/Flat79.png", "start": 254157186, "end": 254159707}, {"filename": "/GameData/textures/FPSTextures/Flat80.png", "start": 254159707, "end": 254161528}, {"filename": "/GameData/textures/FPSTextures/Flat81.png", "start": 254161528, "end": 254164116}, {"filename": "/GameData/textures/FPSTextures/Flat82.png", "start": 254164116, "end": 254166483}, {"filename": "/GameData/textures/FPSTextures/Flat83.png", "start": 254166483, "end": 254168481}, {"filename": "/GameData/textures/FPSTextures/Flat84.png", "start": 254168481, "end": 254170697}, {"filename": "/GameData/textures/FPSTextures/Flat85.png", "start": 254170697, "end": 254172791}, {"filename": "/GameData/textures/FPSTextures/Flat86.png", "start": 254172791, "end": 254175044}, {"filename": "/GameData/textures/FPSTextures/Flat87.png", "start": 254175044, "end": 254177829}, {"filename": "/GameData/textures/FPSTextures/Flat88.png", "start": 254177829, "end": 254180762}, {"filename": "/GameData/textures/FPSTextures/Flat89.png", "start": 254180762, "end": 254183620}, {"filename": "/GameData/textures/FPSTextures/Flat90.png", "start": 254183620, "end": 254186640}, {"filename": "/GameData/textures/FPSTextures/Flat91.png", "start": 254186640, "end": 254189090}, {"filename": "/GameData/textures/FPSTextures/Flat92.png", "start": 254189090, "end": 254195523}, {"filename": "/GameData/textures/FPSTextures/Flat93.png", "start": 254195523, "end": 254201529}, {"filename": "/GameData/textures/FPSTextures/Flat94.png", "start": 254201529, "end": 254209335}, {"filename": "/GameData/textures/FPSTextures/Flat95.png", "start": 254209335, "end": 254215932}, {"filename": "/GameData/textures/FPSTextures/Flat96.png", "start": 254215932, "end": 254221734}, {"filename": "/GameData/textures/FPSTextures/Flat97.png", "start": 254221734, "end": 254227923}, {"filename": "/GameData/textures/FPSTextures/Flat98.png", "start": 254227923, "end": 254233915}, {"filename": "/GameData/textures/FPSTextures/Flat99.png", "start": 254233915, "end": 254236177}, {"filename": "/GameData/textures/FPSTextures/Flat_143.png", "start": 254236177, "end": 254238744}, {"filename": "/GameData/textures/FPSTextures/Flat_144.png", "start": 254238744, "end": 254241597}, {"filename": "/GameData/textures/FPSTextures/Flat_145.png", "start": 254241597, "end": 254244328}, {"filename": "/GameData/textures/FPSTextures/Flat_146.png", "start": 254244328, "end": 254245995}, {"filename": "/GameData/textures/FPSTextures/Flat_147.png", "start": 254245995, "end": 254248673}, {"filename": "/GameData/textures/FPSTextures/Flat_148.png", "start": 254248673, "end": 254251169}, {"filename": "/GameData/textures/FPSTextures/Flat_149.png", "start": 254251169, "end": 254253528}, {"filename": "/GameData/textures/FPSTextures/Flat_150.png", "start": 254253528, "end": 254255795}, {"filename": "/GameData/textures/FPSTextures/Flat_151.png", "start": 254255795, "end": 254258464}, {"filename": "/GameData/textures/FPSTextures/Flat_152.png", "start": 254258464, "end": 254260972}, {"filename": "/GameData/textures/FPSTextures/Flat_153.png", "start": 254260972, "end": 254263981}, {"filename": "/GameData/textures/FPSTextures/Flat_154.png", "start": 254263981, "end": 254266033}, {"filename": "/GameData/textures/FPSTextures/Flat_155.png", "start": 254266033, "end": 254268931}, {"filename": "/GameData/textures/FPSTextures/Flat_156.png", "start": 254268931, "end": 254271772}, {"filename": "/GameData/textures/FPSTextures/Flat_157.png", "start": 254271772, "end": 254274906}, {"filename": "/GameData/textures/FPSTextures/Flat_158.png", "start": 254274906, "end": 254278050}, {"filename": "/GameData/textures/FPSTextures/Flat_159.png", "start": 254278050, "end": 254280292}, {"filename": "/GameData/textures/FPSTextures/Flat_160.png", "start": 254280292, "end": 254282741}, {"filename": "/GameData/textures/FPSTextures/Flat_161.png", "start": 254282741, "end": 254286091}, {"filename": "/GameData/textures/FPSTextures/Flat_162.png", "start": 254286091, "end": 254288464}, {"filename": "/GameData/textures/FPSTextures/Flat_163.png", "start": 254288464, "end": 254291555}, {"filename": "/GameData/textures/FPSTextures/Flat_164.png", "start": 254291555, "end": 254293707}, {"filename": "/GameData/textures/FPSTextures/Flat_165.png", "start": 254293707, "end": 254295867}, {"filename": "/GameData/textures/FPSTextures/Flat_166.png", "start": 254295867, "end": 254298519}, {"filename": "/GameData/textures/FPSTextures/Flat_167.png", "start": 254298519, "end": 254300792}, {"filename": "/GameData/textures/FPSTextures/Flat_168.png", "start": 254300792, "end": 254303210}, {"filename": "/GameData/textures/FPSTextures/Flat_169.png", "start": 254303210, "end": 254305967}, {"filename": "/GameData/textures/FPSTextures/Flat_170.png", "start": 254305967, "end": 254308792}, {"filename": "/GameData/textures/FPSTextures/Flat_171.png", "start": 254308792, "end": 254311623}, {"filename": "/GameData/textures/FPSTextures/Flat_173.png", "start": 254311623, "end": 254313653}, {"filename": "/GameData/textures/FPSTextures/Flat_175.png", "start": 254313653, "end": 254315736}, {"filename": "/GameData/textures/FPSTextures/Flat_176.png", "start": 254315736, "end": 254317822}, {"filename": "/GameData/textures/FPSTextures/Flat_177.png", "start": 254317822, "end": 254319020}, {"filename": "/GameData/textures/FPSTextures/Flat_178.png", "start": 254319020, "end": 254320534}, {"filename": "/GameData/textures/FPSTextures/Flat_179.png", "start": 254320534, "end": 254323017}, {"filename": "/GameData/textures/FPSTextures/Flat_180.png", "start": 254323017, "end": 254325686}, {"filename": "/GameData/textures/FPSTextures/Flat_181.png", "start": 254325686, "end": 254327478}, {"filename": "/GameData/textures/FPSTextures/Flat_182.png", "start": 254327478, "end": 254329678}, {"filename": "/GameData/textures/FPSTextures/Flat_183.png", "start": 254329678, "end": 254332221}, {"filename": "/GameData/textures/FPSTextures/Flat_184.png", "start": 254332221, "end": 254334811}, {"filename": "/GameData/textures/FPSTextures/Flat_185.png", "start": 254334811, "end": 254337701}, {"filename": "/GameData/textures/FPSTextures/Flat_186.png", "start": 254337701, "end": 254341546}, {"filename": "/GameData/textures/FPSTextures/Flat_187.png", "start": 254341546, "end": 254343497}, {"filename": "/GameData/textures/FPSTextures/Flat_188.png", "start": 254343497, "end": 254345999}, {"filename": "/GameData/textures/FPSTextures/Flat_189.png", "start": 254345999, "end": 254348335}, {"filename": "/GameData/textures/FPSTextures/Flat_190.png", "start": 254348335, "end": 254350383}, {"filename": "/GameData/textures/FPSTextures/Flat_191.png", "start": 254350383, "end": 254353063}, {"filename": "/GameData/textures/FPSTextures/Flat_192.png", "start": 254353063, "end": 254355679}, {"filename": "/GameData/textures/FPSTextures/Flat_193.png", "start": 254355679, "end": 254357488}, {"filename": "/GameData/textures/FPSTextures/License.txt", "start": 254357488, "end": 254373219}, {"filename": "/GameData/textures/FPSTextures/Light01.png", "start": 254373219, "end": 254378279}, {"filename": "/GameData/textures/FPSTextures/Light02.png", "start": 254378279, "end": 254383626}, {"filename": "/GameData/textures/FPSTextures/Light03.png", "start": 254383626, "end": 254389530}, {"filename": "/GameData/textures/FPSTextures/Light04.png", "start": 254389530, "end": 254395348}, {"filename": "/GameData/textures/FPSTextures/Light05.png", "start": 254395348, "end": 254397646}, {"filename": "/GameData/textures/FPSTextures/Light06.png", "start": 254397646, "end": 254400644}, {"filename": "/GameData/textures/FPSTextures/Light07.png", "start": 254400644, "end": 254403624}, {"filename": "/GameData/textures/FPSTextures/Light08.png", "start": 254403624, "end": 254406692}, {"filename": "/GameData/textures/FPSTextures/Light09.png", "start": 254406692, "end": 254409438}, {"filename": "/GameData/textures/FPSTextures/Light10.png", "start": 254409438, "end": 254410784}, {"filename": "/GameData/textures/FPSTextures/Metal_01.png", "start": 254410784, "end": 254417586}, {"filename": "/GameData/textures/FPSTextures/Metal_02.png", "start": 254417586, "end": 254425507}, {"filename": "/GameData/textures/FPSTextures/Step01.png", "start": 254425507, "end": 254426700}, {"filename": "/GameData/textures/FPSTextures/Step02.png", "start": 254426700, "end": 254429441}, {"filename": "/GameData/textures/FPSTextures/Step03.png", "start": 254429441, "end": 254432201}, {"filename": "/GameData/textures/FPSTextures/Step04.png", "start": 254432201, "end": 254434962}, {"filename": "/GameData/textures/FPSTextures/Step05.png", "start": 254434962, "end": 254437711}, {"filename": "/GameData/textures/FPSTextures/Step06.png", "start": 254437711, "end": 254438746}, {"filename": "/GameData/textures/FPSTextures/Step07.png", "start": 254438746, "end": 254439963}, {"filename": "/GameData/textures/FPSTextures/Step08.png", "start": 254439963, "end": 254441101}, {"filename": "/GameData/textures/FPSTextures/Strip01.png", "start": 254441101, "end": 254444434}, {"filename": "/GameData/textures/FPSTextures/Strip02.png", "start": 254444434, "end": 254447923}, {"filename": "/GameData/textures/FPSTextures/Strip03.png", "start": 254447923, "end": 254450467}, {"filename": "/GameData/textures/FPSTextures/Strip04.png", "start": 254450467, "end": 254451793}, {"filename": "/GameData/textures/FPSTextures/Strip05.png", "start": 254451793, "end": 254455878}, {"filename": "/GameData/textures/FPSTextures/Strip06.png", "start": 254455878, "end": 254460518}, {"filename": "/GameData/textures/FPSTextures/Strip07.png", "start": 254460518, "end": 254462054}, {"filename": "/GameData/textures/FPSTextures/Wall_01.png", "start": 254462054, "end": 254466956}, {"filename": "/GameData/textures/FPSTextures/Wall_02.png", "start": 254466956, "end": 254474324}, {"filename": "/GameData/textures/FPSTextures/Wall_03.png", "start": 254474324, "end": 254478042}, {"filename": "/GameData/textures/FPSTextures/Wall_04.png", "start": 254478042, "end": 254483643}, {"filename": "/GameData/textures/FPSTextures/Wall_05.png", "start": 254483643, "end": 254487619}, {"filename": "/GameData/textures/FPSTextures/Wall_06.png", "start": 254487619, "end": 254491141}, {"filename": "/GameData/textures/FPSTextures/Wall_07.png", "start": 254491141, "end": 254496125}, {"filename": "/GameData/textures/FPSTextures/Wall_08.png", "start": 254496125, "end": 254505807}, {"filename": "/GameData/textures/FPSTextures/Wall_09.png", "start": 254505807, "end": 254511194}, {"filename": "/GameData/textures/FPSTextures/Wall_10.png", "start": 254511194, "end": 254517603}, {"filename": "/GameData/textures/FPSTextures/Wall_100.png", "start": 254517603, "end": 254520524}, {"filename": "/GameData/textures/FPSTextures/Wall_101.png", "start": 254520524, "end": 254524028}, {"filename": "/GameData/textures/FPSTextures/Wall_102.png", "start": 254524028, "end": 254528501}, {"filename": "/GameData/textures/FPSTextures/Wall_103.png", "start": 254528501, "end": 254530737}, {"filename": "/GameData/textures/FPSTextures/Wall_104.png", "start": 254530737, "end": 254532518}, {"filename": "/GameData/textures/FPSTextures/Wall_105.png", "start": 254532518, "end": 254537401}, {"filename": "/GameData/textures/FPSTextures/Wall_106.png", "start": 254537401, "end": 254540762}, {"filename": "/GameData/textures/FPSTextures/Wall_107.png", "start": 254540762, "end": 254545143}, {"filename": "/GameData/textures/FPSTextures/Wall_108.png", "start": 254545143, "end": 254550625}, {"filename": "/GameData/textures/FPSTextures/Wall_109.png", "start": 254550625, "end": 254554201}, {"filename": "/GameData/textures/FPSTextures/Wall_11.png", "start": 254554201, "end": 254557461}, {"filename": "/GameData/textures/FPSTextures/Wall_110.png", "start": 254557461, "end": 254562154}, {"filename": "/GameData/textures/FPSTextures/Wall_111.png", "start": 254562154, "end": 254572735}, {"filename": "/GameData/textures/FPSTextures/Wall_112.png", "start": 254572735, "end": 254576418}, {"filename": "/GameData/textures/FPSTextures/Wall_113.png", "start": 254576418, "end": 254579678}, {"filename": "/GameData/textures/FPSTextures/Wall_114.png", "start": 254579678, "end": 254582085}, {"filename": "/GameData/textures/FPSTextures/Wall_115.png", "start": 254582085, "end": 254584689}, {"filename": "/GameData/textures/FPSTextures/Wall_116.png", "start": 254584689, "end": 254587206}, {"filename": "/GameData/textures/FPSTextures/Wall_117.png", "start": 254587206, "end": 254593120}, {"filename": "/GameData/textures/FPSTextures/Wall_118.png", "start": 254593120, "end": 254597160}, {"filename": "/GameData/textures/FPSTextures/Wall_119.png", "start": 254597160, "end": 254607372}, {"filename": "/GameData/textures/FPSTextures/Wall_12.png", "start": 254607372, "end": 254610785}, {"filename": "/GameData/textures/FPSTextures/Wall_120.png", "start": 254610785, "end": 254613252}, {"filename": "/GameData/textures/FPSTextures/Wall_121.png", "start": 254613252, "end": 254616488}, {"filename": "/GameData/textures/FPSTextures/Wall_122.png", "start": 254616488, "end": 254620084}, {"filename": "/GameData/textures/FPSTextures/Wall_123.png", "start": 254620084, "end": 254622928}, {"filename": "/GameData/textures/FPSTextures/Wall_124.png", "start": 254622928, "end": 254625125}, {"filename": "/GameData/textures/FPSTextures/Wall_125.png", "start": 254625125, "end": 254630471}, {"filename": "/GameData/textures/FPSTextures/Wall_126.png", "start": 254630471, "end": 254633142}, {"filename": "/GameData/textures/FPSTextures/Wall_127.png", "start": 254633142, "end": 254635846}, {"filename": "/GameData/textures/FPSTextures/Wall_128.png", "start": 254635846, "end": 254638468}, {"filename": "/GameData/textures/FPSTextures/Wall_129.png", "start": 254638468, "end": 254642045}, {"filename": "/GameData/textures/FPSTextures/Wall_13.png", "start": 254642045, "end": 254645402}, {"filename": "/GameData/textures/FPSTextures/Wall_130.png", "start": 254645402, "end": 254648183}, {"filename": "/GameData/textures/FPSTextures/Wall_131.png", "start": 254648183, "end": 254649757}, {"filename": "/GameData/textures/FPSTextures/Wall_132.png", "start": 254649757, "end": 254654639}, {"filename": "/GameData/textures/FPSTextures/Wall_133.png", "start": 254654639, "end": 254659057}, {"filename": "/GameData/textures/FPSTextures/Wall_134.png", "start": 254659057, "end": 254664630}, {"filename": "/GameData/textures/FPSTextures/Wall_135.png", "start": 254664630, "end": 254668345}, {"filename": "/GameData/textures/FPSTextures/Wall_136.png", "start": 254668345, "end": 254673113}, {"filename": "/GameData/textures/FPSTextures/Wall_137.png", "start": 254673113, "end": 254677730}, {"filename": "/GameData/textures/FPSTextures/Wall_138.png", "start": 254677730, "end": 254682343}, {"filename": "/GameData/textures/FPSTextures/Wall_139.png", "start": 254682343, "end": 254686257}, {"filename": "/GameData/textures/FPSTextures/Wall_14.png", "start": 254686257, "end": 254690097}, {"filename": "/GameData/textures/FPSTextures/Wall_140.png", "start": 254690097, "end": 254694057}, {"filename": "/GameData/textures/FPSTextures/Wall_141.png", "start": 254694057, "end": 254697356}, {"filename": "/GameData/textures/FPSTextures/Wall_142.png", "start": 254697356, "end": 254700067}, {"filename": "/GameData/textures/FPSTextures/Wall_143.png", "start": 254700067, "end": 254704561}, {"filename": "/GameData/textures/FPSTextures/Wall_144.png", "start": 254704561, "end": 254706783}, {"filename": "/GameData/textures/FPSTextures/Wall_145.png", "start": 254706783, "end": 254709684}, {"filename": "/GameData/textures/FPSTextures/Wall_146.png", "start": 254709684, "end": 254712429}, {"filename": "/GameData/textures/FPSTextures/Wall_147.png", "start": 254712429, "end": 254715779}, {"filename": "/GameData/textures/FPSTextures/Wall_148.png", "start": 254715779, "end": 254719200}, {"filename": "/GameData/textures/FPSTextures/Wall_149.png", "start": 254719200, "end": 254722470}, {"filename": "/GameData/textures/FPSTextures/Wall_15.png", "start": 254722470, "end": 254727600}, {"filename": "/GameData/textures/FPSTextures/Wall_150.png", "start": 254727600, "end": 254732871}, {"filename": "/GameData/textures/FPSTextures/Wall_151.png", "start": 254732871, "end": 254737078}, {"filename": "/GameData/textures/FPSTextures/Wall_152.png", "start": 254737078, "end": 254741638}, {"filename": "/GameData/textures/FPSTextures/Wall_153.png", "start": 254741638, "end": 254745914}, {"filename": "/GameData/textures/FPSTextures/Wall_154.png", "start": 254745914, "end": 254748359}, {"filename": "/GameData/textures/FPSTextures/Wall_155.png", "start": 254748359, "end": 254750619}, {"filename": "/GameData/textures/FPSTextures/Wall_156.png", "start": 254750619, "end": 254754320}, {"filename": "/GameData/textures/FPSTextures/Wall_157.png", "start": 254754320, "end": 254757187}, {"filename": "/GameData/textures/FPSTextures/Wall_158.png", "start": 254757187, "end": 254760557}, {"filename": "/GameData/textures/FPSTextures/Wall_159.png", "start": 254760557, "end": 254762648}, {"filename": "/GameData/textures/FPSTextures/Wall_16.png", "start": 254762648, "end": 254766213}, {"filename": "/GameData/textures/FPSTextures/Wall_160.png", "start": 254766213, "end": 254768948}, {"filename": "/GameData/textures/FPSTextures/Wall_161.png", "start": 254768948, "end": 254772033}, {"filename": "/GameData/textures/FPSTextures/Wall_162.png", "start": 254772033, "end": 254776106}, {"filename": "/GameData/textures/FPSTextures/Wall_163.png", "start": 254776106, "end": 254780093}, {"filename": "/GameData/textures/FPSTextures/Wall_164.png", "start": 254780093, "end": 254782854}, {"filename": "/GameData/textures/FPSTextures/Wall_166.png", "start": 254782854, "end": 254785446}, {"filename": "/GameData/textures/FPSTextures/Wall_167.png", "start": 254785446, "end": 254789984}, {"filename": "/GameData/textures/FPSTextures/Wall_168.png", "start": 254789984, "end": 254792996}, {"filename": "/GameData/textures/FPSTextures/Wall_169.png", "start": 254792996, "end": 254795432}, {"filename": "/GameData/textures/FPSTextures/Wall_17.png", "start": 254795432, "end": 254802531}, {"filename": "/GameData/textures/FPSTextures/Wall_170.png", "start": 254802531, "end": 254805445}, {"filename": "/GameData/textures/FPSTextures/Wall_171.png", "start": 254805445, "end": 254809810}, {"filename": "/GameData/textures/FPSTextures/Wall_172.png", "start": 254809810, "end": 254812023}, {"filename": "/GameData/textures/FPSTextures/Wall_173.png", "start": 254812023, "end": 254815970}, {"filename": "/GameData/textures/FPSTextures/Wall_174.png", "start": 254815970, "end": 254819850}, {"filename": "/GameData/textures/FPSTextures/Wall_175.png", "start": 254819850, "end": 254824435}, {"filename": "/GameData/textures/FPSTextures/Wall_178.png", "start": 254824435, "end": 254835672}, {"filename": "/GameData/textures/FPSTextures/Wall_179.png", "start": 254835672, "end": 254842739}, {"filename": "/GameData/textures/FPSTextures/Wall_18.png", "start": 254842739, "end": 254849649}, {"filename": "/GameData/textures/FPSTextures/Wall_180.png", "start": 254849649, "end": 254853821}, {"filename": "/GameData/textures/FPSTextures/Wall_181.png", "start": 254853821, "end": 254858144}, {"filename": "/GameData/textures/FPSTextures/Wall_182.png", "start": 254858144, "end": 254861496}, {"filename": "/GameData/textures/FPSTextures/Wall_183.png", "start": 254861496, "end": 254864631}, {"filename": "/GameData/textures/FPSTextures/Wall_184.png", "start": 254864631, "end": 254868011}, {"filename": "/GameData/textures/FPSTextures/Wall_185.png", "start": 254868011, "end": 254870958}, {"filename": "/GameData/textures/FPSTextures/Wall_186.png", "start": 254870958, "end": 254874293}, {"filename": "/GameData/textures/FPSTextures/Wall_187.png", "start": 254874293, "end": 254877686}, {"filename": "/GameData/textures/FPSTextures/Wall_188.png", "start": 254877686, "end": 254880478}, {"filename": "/GameData/textures/FPSTextures/Wall_19.png", "start": 254880478, "end": 254888077}, {"filename": "/GameData/textures/FPSTextures/Wall_20.png", "start": 254888077, "end": 254894671}, {"filename": "/GameData/textures/FPSTextures/Wall_21.png", "start": 254894671, "end": 254899166}, {"filename": "/GameData/textures/FPSTextures/Wall_22.png", "start": 254899166, "end": 254905020}, {"filename": "/GameData/textures/FPSTextures/Wall_23.png", "start": 254905020, "end": 254908038}, {"filename": "/GameData/textures/FPSTextures/Wall_24.png", "start": 254908038, "end": 254915044}, {"filename": "/GameData/textures/FPSTextures/Wall_25.png", "start": 254915044, "end": 254919872}, {"filename": "/GameData/textures/FPSTextures/Wall_26.png", "start": 254919872, "end": 254924062}, {"filename": "/GameData/textures/FPSTextures/Wall_30.png", "start": 254924062, "end": 254927824}, {"filename": "/GameData/textures/FPSTextures/Wall_31.png", "start": 254927824, "end": 254931283}, {"filename": "/GameData/textures/FPSTextures/Wall_32.png", "start": 254931283, "end": 254934704}, {"filename": "/GameData/textures/FPSTextures/Wall_33.png", "start": 254934704, "end": 254943236}, {"filename": "/GameData/textures/FPSTextures/Wall_34.png", "start": 254943236, "end": 254948180}, {"filename": "/GameData/textures/FPSTextures/Wall_35.png", "start": 254948180, "end": 254954439}, {"filename": "/GameData/textures/FPSTextures/Wall_36.png", "start": 254954439, "end": 254959185}, {"filename": "/GameData/textures/FPSTextures/Wall_37.png", "start": 254959185, "end": 254966123}, {"filename": "/GameData/textures/FPSTextures/Wall_38.png", "start": 254966123, "end": 254974938}, {"filename": "/GameData/textures/FPSTextures/Wall_39.png", "start": 254974938, "end": 254981564}, {"filename": "/GameData/textures/FPSTextures/Wall_40.png", "start": 254981564, "end": 254983254}, {"filename": "/GameData/textures/FPSTextures/Wall_41.png", "start": 254983254, "end": 254990733}, {"filename": "/GameData/textures/FPSTextures/Wall_42.png", "start": 254990733, "end": 254998346}, {"filename": "/GameData/textures/FPSTextures/Wall_43.png", "start": 254998346, "end": 255005312}, {"filename": "/GameData/textures/FPSTextures/Wall_44.png", "start": 255005312, "end": 255008353}, {"filename": "/GameData/textures/FPSTextures/Wall_45.png", "start": 255008353, "end": 255012004}, {"filename": "/GameData/textures/FPSTextures/Wall_46.png", "start": 255012004, "end": 255020730}, {"filename": "/GameData/textures/FPSTextures/Wall_47.png", "start": 255020730, "end": 255028364}, {"filename": "/GameData/textures/FPSTextures/Wall_48.png", "start": 255028364, "end": 255031742}, {"filename": "/GameData/textures/FPSTextures/Wall_49.png", "start": 255031742, "end": 255039445}, {"filename": "/GameData/textures/FPSTextures/Wall_50.png", "start": 255039445, "end": 255042822}, {"filename": "/GameData/textures/FPSTextures/Wall_51.png", "start": 255042822, "end": 255046623}, {"filename": "/GameData/textures/FPSTextures/Wall_52.png", "start": 255046623, "end": 255054128}, {"filename": "/GameData/textures/FPSTextures/Wall_53.png", "start": 255054128, "end": 255057934}, {"filename": "/GameData/textures/FPSTextures/Wall_54.png", "start": 255057934, "end": 255060923}, {"filename": "/GameData/textures/FPSTextures/Wall_55.png", "start": 255060923, "end": 255070887}, {"filename": "/GameData/textures/FPSTextures/Wall_56.png", "start": 255070887, "end": 255078205}, {"filename": "/GameData/textures/FPSTextures/Wall_57.png", "start": 255078205, "end": 255082558}, {"filename": "/GameData/textures/FPSTextures/Wall_58.png", "start": 255082558, "end": 255087205}, {"filename": "/GameData/textures/FPSTextures/Wall_59.png", "start": 255087205, "end": 255090301}, {"filename": "/GameData/textures/FPSTextures/Wall_61.png", "start": 255090301, "end": 255093479}, {"filename": "/GameData/textures/FPSTextures/Wall_62.png", "start": 255093479, "end": 255096596}, {"filename": "/GameData/textures/FPSTextures/Wall_63.png", "start": 255096596, "end": 255100716}, {"filename": "/GameData/textures/FPSTextures/Wall_64.png", "start": 255100716, "end": 255108184}, {"filename": "/GameData/textures/FPSTextures/Wall_65.png", "start": 255108184, "end": 255116130}, {"filename": "/GameData/textures/FPSTextures/Wall_66.png", "start": 255116130, "end": 255122977}, {"filename": "/GameData/textures/FPSTextures/Wall_67.png", "start": 255122977, "end": 255129904}, {"filename": "/GameData/textures/FPSTextures/Wall_68.png", "start": 255129904, "end": 255137242}, {"filename": "/GameData/textures/FPSTextures/Wall_69.png", "start": 255137242, "end": 255143488}, {"filename": "/GameData/textures/FPSTextures/Wall_70.png", "start": 255143488, "end": 255149914}, {"filename": "/GameData/textures/FPSTextures/Wall_71.png", "start": 255149914, "end": 255156977}, {"filename": "/GameData/textures/FPSTextures/Wall_72.png", "start": 255156977, "end": 255164475}, {"filename": "/GameData/textures/FPSTextures/Wall_73.png", "start": 255164475, "end": 255169683}, {"filename": "/GameData/textures/FPSTextures/Wall_74.png", "start": 255169683, "end": 255171823}, {"filename": "/GameData/textures/FPSTextures/Wall_75.png", "start": 255171823, "end": 255175578}, {"filename": "/GameData/textures/FPSTextures/Wall_76.png", "start": 255175578, "end": 255179592}, {"filename": "/GameData/textures/FPSTextures/Wall_77.png", "start": 255179592, "end": 255188328}, {"filename": "/GameData/textures/FPSTextures/Wall_78.png", "start": 255188328, "end": 255197119}, {"filename": "/GameData/textures/FPSTextures/Wall_79.png", "start": 255197119, "end": 255199180}, {"filename": "/GameData/textures/FPSTextures/Wall_80.png", "start": 255199180, "end": 255201459}, {"filename": "/GameData/textures/FPSTextures/Wall_81.png", "start": 255201459, "end": 255203919}, {"filename": "/GameData/textures/FPSTextures/Wall_82.png", "start": 255203919, "end": 255205739}, {"filename": "/GameData/textures/FPSTextures/Wall_83.png", "start": 255205739, "end": 255214471}, {"filename": "/GameData/textures/FPSTextures/Wall_84.png", "start": 255214471, "end": 255217997}, {"filename": "/GameData/textures/FPSTextures/Wall_85.png", "start": 255217997, "end": 255219761}, {"filename": "/GameData/textures/FPSTextures/Wall_86.png", "start": 255219761, "end": 255222634}, {"filename": "/GameData/textures/FPSTextures/Wall_87.png", "start": 255222634, "end": 255229644}, {"filename": "/GameData/textures/FPSTextures/Wall_88.png", "start": 255229644, "end": 255236178}, {"filename": "/GameData/textures/FPSTextures/Wall_89.png", "start": 255236178, "end": 255239922}, {"filename": "/GameData/textures/FPSTextures/Wall_90.png", "start": 255239922, "end": 255244000}, {"filename": "/GameData/textures/FPSTextures/Wall_91.png", "start": 255244000, "end": 255247995}, {"filename": "/GameData/textures/FPSTextures/Wall_92.png", "start": 255247995, "end": 255252208}, {"filename": "/GameData/textures/FPSTextures/Wall_93.png", "start": 255252208, "end": 255256497}, {"filename": "/GameData/textures/FPSTextures/Wall_94.png", "start": 255256497, "end": 255259746}, {"filename": "/GameData/textures/FPSTextures/Wall_95.png", "start": 255259746, "end": 255264654}, {"filename": "/GameData/textures/FPSTextures/Wall_96.png", "start": 255264654, "end": 255272986}, {"filename": "/GameData/textures/FPSTextures/Wall_97.png", "start": 255272986, "end": 255281748}, {"filename": "/GameData/textures/FPSTextures/Wall_98.png", "start": 255281748, "end": 255292360}, {"filename": "/GameData/textures/FPSTextures/Wall_99.png", "start": 255292360, "end": 255299431}, {"filename": "/GameData/textures/FPSTextures/flat_172.png", "start": 255299431, "end": 255301592}, {"filename": "/GameData/textures/FPSTextures/wall_165.png", "start": 255301592, "end": 255304687}, {"filename": "/GameData/textures/FPSTextures_HD/wall_wood_wallpaper.png", "start": 255304687, "end": 255350851}, {"filename": "/GameData/textures/Ground/grass.png", "start": 255350851, "end": 255354635}, {"filename": "/GameData/textures/M_Shotgun_Base_Color.png", "start": 255354635, "end": 255729777}, {"filename": "/GameData/textures/arms.png", "start": 255729777, "end": 255765632}, {"filename": "/GameData/textures/building_1/b1_f1.png", "start": 255765632, "end": 255863656}, {"filename": "/GameData/textures/building_1/b1_f2.png", "start": 255863656, "end": 255938598}, {"filename": "/GameData/textures/building_1/b1_f3.png", "start": 255938598, "end": 256015535}, {"filename": "/GameData/textures/common/trigger.png", "start": 256015535, "end": 256027411}, {"filename": "/GameData/textures/delvenPack/dlv_door1a.png", "start": 256027411, "end": 256035195}, {"filename": "/GameData/textures/delvenPack/dlv_door1b.png", "start": 256035195, "end": 256044385}, {"filename": "/GameData/textures/delvenPack/dlv_door1c.png", "start": 256044385, "end": 256052236}, {"filename": "/GameData/textures/delvenPack/dlv_door1d.png", "start": 256052236, "end": 256061408}, {"filename": "/GameData/textures/delvenPack/dlv_door2a.png", "start": 256061408, "end": 256069132}, {"filename": "/GameData/textures/delvenPack/dlv_door2b.png", "start": 256069132, "end": 256078257}, {"filename": "/GameData/textures/delvenPack/dlv_door2c.png", "start": 256078257, "end": 256086285}, {"filename": "/GameData/textures/delvenPack/dlv_door2d.png", "start": 256086285, "end": 256095513}, {"filename": "/GameData/textures/delvenPack/dlv_door3a.png", "start": 256095513, "end": 256099661}, {"filename": "/GameData/textures/delvenPack/dlv_door3b.png", "start": 256099661, "end": 256104439}, {"filename": "/GameData/textures/delvenPack/dlv_door3c.png", "start": 256104439, "end": 256108589}, {"filename": "/GameData/textures/delvenPack/dlv_door3d.png", "start": 256108589, "end": 256113381}, {"filename": "/GameData/textures/delvenPack/dlv_door4a.png", "start": 256113381, "end": 256117502}, {"filename": "/GameData/textures/delvenPack/dlv_door4b.png", "start": 256117502, "end": 256122293}, {"filename": "/GameData/textures/delvenPack/dlv_door4c.png", "start": 256122293, "end": 256126563}, {"filename": "/GameData/textures/delvenPack/dlv_door4d.png", "start": 256126563, "end": 256131399}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1a.png", "start": 256131399, "end": 256136775}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1b.png", "start": 256136775, "end": 256142903}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1c.png", "start": 256142903, "end": 256149197}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1d.png", "start": 256149197, "end": 256152833}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1e.png", "start": 256152833, "end": 256156519}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2a.png", "start": 256156519, "end": 256161244}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2b.png", "start": 256161244, "end": 256166695}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2c.png", "start": 256166695, "end": 256172373}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2d.png", "start": 256172373, "end": 256175659}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2e.png", "start": 256175659, "end": 256179020}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3a.png", "start": 256179020, "end": 256183974}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3b.png", "start": 256183974, "end": 256189645}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3c.png", "start": 256189645, "end": 256195470}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3d.png", "start": 256195470, "end": 256198850}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3e.png", "start": 256198850, "end": 256202292}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4a.png", "start": 256202292, "end": 256211710}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4b.png", "start": 256211710, "end": 256221675}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4c.png", "start": 256221675, "end": 256231804}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4d.png", "start": 256231804, "end": 256237328}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4e.png", "start": 256237328, "end": 256242898}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5a.png", "start": 256242898, "end": 256251965}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5b.png", "start": 256251965, "end": 256261612}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5c.png", "start": 256261612, "end": 256271412}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5d.png", "start": 256271412, "end": 256276822}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5e.png", "start": 256276822, "end": 256282256}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6a.png", "start": 256282256, "end": 256289390}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6b.png", "start": 256289390, "end": 256297118}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6c.png", "start": 256297118, "end": 256305013}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6d.png", "start": 256305013, "end": 256309413}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6e.png", "start": 256309413, "end": 256313871}, {"filename": "/GameData/textures/delvenPack/dlv_ground1a.png", "start": 256313871, "end": 256321793}, {"filename": "/GameData/textures/delvenPack/dlv_ground1b.png", "start": 256321793, "end": 256330629}, {"filename": "/GameData/textures/delvenPack/dlv_ground1c.png", "start": 256330629, "end": 256340125}, {"filename": "/GameData/textures/delvenPack/dlv_ground1d.png", "start": 256340125, "end": 256350168}, {"filename": "/GameData/textures/delvenPack/dlv_ground2a.png", "start": 256350168, "end": 256360036}, {"filename": "/GameData/textures/delvenPack/dlv_ground2b.png", "start": 256360036, "end": 256368943}, {"filename": "/GameData/textures/delvenPack/dlv_ground2c.png", "start": 256368943, "end": 256378668}, {"filename": "/GameData/textures/delvenPack/dlv_ground2d.png", "start": 256378668, "end": 256389089}, {"filename": "/GameData/textures/delvenPack/dlv_ground3a.png", "start": 256389089, "end": 256399148}, {"filename": "/GameData/textures/delvenPack/dlv_ground3b.png", "start": 256399148, "end": 256408860}, {"filename": "/GameData/textures/delvenPack/dlv_ground3c.png", "start": 256408860, "end": 256418296}, {"filename": "/GameData/textures/delvenPack/dlv_ground4a.png", "start": 256418296, "end": 256428487}, {"filename": "/GameData/textures/delvenPack/dlv_ground4b.png", "start": 256428487, "end": 256438937}, {"filename": "/GameData/textures/delvenPack/dlv_ground4c.png", "start": 256438937, "end": 256448878}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal1a.png", "start": 256448878, "end": 256452906}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal1b.png", "start": 256452906, "end": 256457618}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal1c.png", "start": 256457618, "end": 256461699}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal2a.png", "start": 256461699, "end": 256465752}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal2b.png", "start": 256465752, "end": 256470511}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal2c.png", "start": 256470511, "end": 256474382}, {"filename": "/GameData/textures/delvenPack/dlv_litstone1a.png", "start": 256474382, "end": 256477760}, {"filename": "/GameData/textures/delvenPack/dlv_litstone1b.png", "start": 256477760, "end": 256481973}, {"filename": "/GameData/textures/delvenPack/dlv_litstone1c.png", "start": 256481973, "end": 256485604}, {"filename": "/GameData/textures/delvenPack/dlv_litstone2a.png", "start": 256485604, "end": 256490401}, {"filename": "/GameData/textures/delvenPack/dlv_litstone2b.png", "start": 256490401, "end": 256495927}, {"filename": "/GameData/textures/delvenPack/dlv_litstone2c.png", "start": 256495927, "end": 256500170}, {"filename": "/GameData/textures/delvenPack/dlv_metalgen1.png", "start": 256500170, "end": 256505995}, {"filename": "/GameData/textures/delvenPack/dlv_metalgen2.png", "start": 256505995, "end": 256512875}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan1a.png", "start": 256512875, "end": 256520933}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan1b.png", "start": 256520933, "end": 256528403}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan1c.png", "start": 256528403, "end": 256537605}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan2a.png", "start": 256537605, "end": 256545932}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan2b.png", "start": 256545932, "end": 256553344}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan2c.png", "start": 256553344, "end": 256562543}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan3a.png", "start": 256562543, "end": 256571150}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan3b.png", "start": 256571150, "end": 256579377}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan3c.png", "start": 256579377, "end": 256588679}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan4a.png", "start": 256588679, "end": 256597440}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan4b.png", "start": 256597440, "end": 256605765}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan4c.png", "start": 256605765, "end": 256615220}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip1a.png", "start": 256615220, "end": 256621864}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip1b.png", "start": 256621864, "end": 256628376}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip2a.png", "start": 256628376, "end": 256635509}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip2b.png", "start": 256635509, "end": 256642527}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip3a.png", "start": 256642527, "end": 256649798}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip3b.png", "start": 256649798, "end": 256656876}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip4a.png", "start": 256656876, "end": 256664444}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip4b.png", "start": 256664444, "end": 256671799}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm1a.png", "start": 256671799, "end": 256678797}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm1b.png", "start": 256678797, "end": 256685330}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm1c.png", "start": 256685330, "end": 256693518}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm2a.png", "start": 256693518, "end": 256701015}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm2b.png", "start": 256701015, "end": 256707603}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm2c.png", "start": 256707603, "end": 256715864}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm3a.png", "start": 256715864, "end": 256723140}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm3b.png", "start": 256723140, "end": 256730066}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm3c.png", "start": 256730066, "end": 256738061}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm4a.png", "start": 256738061, "end": 256744941}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm4b.png", "start": 256744941, "end": 256751649}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm4c.png", "start": 256751649, "end": 256759458}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm5.png", "start": 256759458, "end": 256763117}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm6.png", "start": 256763117, "end": 256766840}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1a.png", "start": 256766840, "end": 256773632}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1b.png", "start": 256773632, "end": 256782003}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1c.png", "start": 256782003, "end": 256789673}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1d.png", "start": 256789673, "end": 256797790}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1e.png", "start": 256797790, "end": 256805682}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2a.png", "start": 256805682, "end": 256813873}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2b.png", "start": 256813873, "end": 256823385}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2c.png", "start": 256823385, "end": 256832287}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2d.png", "start": 256832287, "end": 256841538}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2e.png", "start": 256841538, "end": 256850609}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3a.png", "start": 256850609, "end": 256858740}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3b.png", "start": 256858740, "end": 256868450}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3c.png", "start": 256868450, "end": 256877422}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3d.png", "start": 256877422, "end": 256886896}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3e.png", "start": 256886896, "end": 256896123}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr4a.png", "start": 256896123, "end": 256904629}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr4b.png", "start": 256904629, "end": 256913089}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr4c.png", "start": 256913089, "end": 256921753}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr5a.png", "start": 256921753, "end": 256928932}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr5b.png", "start": 256928932, "end": 256936060}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr5c.png", "start": 256936060, "end": 256944200}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr6a.png", "start": 256944200, "end": 256952876}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr6b.png", "start": 256952876, "end": 256961291}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr6c.png", "start": 256961291, "end": 256970024}, {"filename": "/GameData/textures/delvenPack/dlv_slategen1.png", "start": 256970024, "end": 256975985}, {"filename": "/GameData/textures/delvenPack/dlv_slategen2.png", "start": 256975985, "end": 256983544}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1a.png", "start": 256983544, "end": 256989494}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1b.png", "start": 256989494, "end": 256995310}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1c.png", "start": 256995310, "end": 257001254}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1d.png", "start": 257001254, "end": 257006900}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2a.png", "start": 257006900, "end": 257015772}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2b.png", "start": 257015772, "end": 257024446}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2c.png", "start": 257024446, "end": 257033243}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2d.png", "start": 257033243, "end": 257041539}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk3a.png", "start": 257041539, "end": 257049986}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk3b.png", "start": 257049986, "end": 257058633}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4a.png", "start": 257058633, "end": 257065252}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4b.png", "start": 257065252, "end": 257071938}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4c.png", "start": 257071938, "end": 257078555}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4d.png", "start": 257078555, "end": 257085169}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4e.png", "start": 257085169, "end": 257091753}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4f.png", "start": 257091753, "end": 257098110}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5a.png", "start": 257098110, "end": 257107587}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5b.png", "start": 257107587, "end": 257117099}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5c.png", "start": 257117099, "end": 257126597}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5d.png", "start": 257126597, "end": 257136070}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5e.png", "start": 257136070, "end": 257145461}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5f.png", "start": 257145461, "end": 257154598}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk6a.png", "start": 257154598, "end": 257163523}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk6b.png", "start": 257163523, "end": 257172748}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr1a.png", "start": 257172748, "end": 257178178}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr1b.png", "start": 257178178, "end": 257184600}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr1c.png", "start": 257184600, "end": 257191623}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr2a.png", "start": 257191623, "end": 257199511}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr2b.png", "start": 257199511, "end": 257208326}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr2c.png", "start": 257208326, "end": 257217607}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr3a.png", "start": 257217607, "end": 257223310}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr3b.png", "start": 257223310, "end": 257229788}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr3c.png", "start": 257229788, "end": 257236780}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr4a.png", "start": 257236780, "end": 257245172}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr4b.png", "start": 257245172, "end": 257254241}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr4c.png", "start": 257254241, "end": 257263341}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr5a.png", "start": 257263341, "end": 257269431}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr5b.png", "start": 257269431, "end": 257275844}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr5c.png", "start": 257275844, "end": 257282762}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr6a.png", "start": 257282762, "end": 257291307}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr6b.png", "start": 257291307, "end": 257300127}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr6c.png", "start": 257300127, "end": 257309411}, {"filename": "/GameData/textures/delvenPack/dlv_stonegen1.png", "start": 257309411, "end": 257314505}, {"filename": "/GameData/textures/delvenPack/dlv_stonegen2.png", "start": 257314505, "end": 257321935}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep1a.png", "start": 257321935, "end": 257327378}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep1b.png", "start": 257327378, "end": 257332918}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep2a.png", "start": 257332918, "end": 257340767}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep2b.png", "start": 257340767, "end": 257348517}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm1a.png", "start": 257348517, "end": 257353807}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm1b.png", "start": 257353807, "end": 257359907}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm1c.png", "start": 257359907, "end": 257365483}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm2a.png", "start": 257365483, "end": 257373229}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm2b.png", "start": 257373229, "end": 257382059}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm2c.png", "start": 257382059, "end": 257390367}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm3a.png", "start": 257390367, "end": 257395667}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm3b.png", "start": 257395667, "end": 257401740}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm4a.png", "start": 257401740, "end": 257409562}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm4b.png", "start": 257409562, "end": 257418093}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm5.png", "start": 257418093, "end": 257421250}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm6.png", "start": 257421250, "end": 257425605}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf1a.png", "start": 257425605, "end": 257431216}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf1b.png", "start": 257431216, "end": 257436833}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf2a.png", "start": 257436833, "end": 257444706}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf2b.png", "start": 257444706, "end": 257452815}, {"filename": "/GameData/textures/delvenPack/dlv_wood1a.png", "start": 257452815, "end": 257457514}, {"filename": "/GameData/textures/delvenPack/dlv_wood1b.png", "start": 257457514, "end": 257462473}, {"filename": "/GameData/textures/delvenPack/dlv_wood2a.png", "start": 257462473, "end": 257469169}, {"filename": "/GameData/textures/delvenPack/dlv_wood2b.png", "start": 257469169, "end": 257476126}, {"filename": "/GameData/textures/delvenPack/dlv_wood3a.png", "start": 257476126, "end": 257481330}, {"filename": "/GameData/textures/delvenPack/dlv_wood3b.png", "start": 257481330, "end": 257486788}, {"filename": "/GameData/textures/delvenPack/dlv_wood4a.png", "start": 257486788, "end": 257493620}, {"filename": "/GameData/textures/delvenPack/dlv_wood4b.png", "start": 257493620, "end": 257500665}, {"filename": "/GameData/textures/delvenPack/dlv_wood5a.png", "start": 257500665, "end": 257506678}, {"filename": "/GameData/textures/delvenPack/dlv_wood5b.png", "start": 257506678, "end": 257513513}, {"filename": "/GameData/textures/delvenPack/dlv_wood5c.png", "start": 257513513, "end": 257520523}, {"filename": "/GameData/textures/delvenPack/dlv_wood5d.png", "start": 257520523, "end": 257528113}, {"filename": "/GameData/textures/delvenPack/dlv_wood5e.png", "start": 257528113, "end": 257537246}, {"filename": "/GameData/textures/delvenPack/dlv_wood5f.png", "start": 257537246, "end": 257546275}, {"filename": "/GameData/textures/delvenPack/dlv_wood5g.png", "start": 257546275, "end": 257555009}, {"filename": "/GameData/textures/delvenPack/dlv_wood5h.png", "start": 257555009, "end": 257563737}, {"filename": "/GameData/textures/delvenPack/dlv_wood6a.png", "start": 257563737, "end": 257571367}, {"filename": "/GameData/textures/delvenPack/dlv_wood6b.png", "start": 257571367, "end": 257579815}, {"filename": "/GameData/textures/delvenPack/dlv_wood6c.png", "start": 257579815, "end": 257587383}, {"filename": "/GameData/textures/delvenPack/dlv_wood6d.png", "start": 257587383, "end": 257595780}, {"filename": "/GameData/textures/delvenPack/dlv_wood6e.png", "start": 257595780, "end": 257605537}, {"filename": "/GameData/textures/delvenPack/dlv_wood6f.png", "start": 257605537, "end": 257615483}, {"filename": "/GameData/textures/delvenPack/dlv_wood6g.png", "start": 257615483, "end": 257624686}, {"filename": "/GameData/textures/delvenPack/dlv_wood6h.png", "start": 257624686, "end": 257634125}, {"filename": "/GameData/textures/delvenPack/dlv_woodgen1.png", "start": 257634125, "end": 257638672}, {"filename": "/GameData/textures/delvenPack/dlv_woodgen2.png", "start": 257638672, "end": 257645365}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1a.png", "start": 257645365, "end": 257653152}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1b.png", "start": 257653152, "end": 257660299}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1c.png", "start": 257660299, "end": 257664569}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1d.png", "start": 257664569, "end": 257668579}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2a.png", "start": 257668579, "end": 257675718}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2b.png", "start": 257675718, "end": 257682277}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2c.png", "start": 257682277, "end": 257686098}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2d.png", "start": 257686098, "end": 257689736}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3a.png", "start": 257689736, "end": 257696865}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3b.png", "start": 257696865, "end": 257703451}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3c.png", "start": 257703451, "end": 257707424}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3d.png", "start": 257707424, "end": 257711196}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4a.png", "start": 257711196, "end": 257722078}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4b.png", "start": 257722078, "end": 257730916}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4c.png", "start": 257730916, "end": 257736418}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4d.png", "start": 257736418, "end": 257741128}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5a.png", "start": 257741128, "end": 257751712}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5b.png", "start": 257751712, "end": 257760338}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5c.png", "start": 257760338, "end": 257765647}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5d.png", "start": 257765647, "end": 257770197}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6a.png", "start": 257770197, "end": 257779046}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6b.png", "start": 257779046, "end": 257786324}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6c.png", "start": 257786324, "end": 257790942}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6d.png", "start": 257790942, "end": 257794954}, {"filename": "/GameData/textures/generic/__TB_empty.png", "start": 257794954, "end": 257795712}, {"filename": "/GameData/textures/generic/brick.png", "start": 257795712, "end": 258308626}, {"filename": "/GameData/textures/generic/brickPBR.png", "start": 258308626, "end": 259238758}, {"filename": "/GameData/textures/generic/brickPBR_orm.png", "start": 259238758, "end": 259663609}, {"filename": "/GameData/textures/generic/bricks.png", "start": 259663609, "end": 259674529}, {"filename": "/GameData/textures/generic/cat.png", "start": 259674529, "end": 259923967}, {"filename": "/GameData/textures/generic/foil.png", "start": 259923967, "end": 260181185}, {"filename": "/GameData/textures/generic/grass.png", "start": 260181185, "end": 260309310}, {"filename": "/GameData/textures/generic/hole_t.png", "start": 260309310, "end": 260311739}, {"filename": "/GameData/textures/generic/light.png", "start": 260311739, "end": 260311867}, {"filename": "/GameData/textures/generic/light_em.png", "start": 260311867, "end": 260311995}, {"filename": "/GameData/textures/generic/mask_test_m.png", "start": 260311995, "end": 260316057}, {"filename": "/GameData/textures/generic/mirror.png", "start": 260316057, "end": 260316177}, {"filename": "/GameData/textures/generic/mirror_orm.png", "start": 260316177, "end": 260316297}, {"filename": "/GameData/textures/generic/null_m.png", "start": 260316297, "end": 260318094}, {"filename": "/GameData/textures/generic/trigger_t.png", "start": 260318094, "end": 260329970}, {"filename": "/GameData/textures/generic/white.png", "start": 260329970, "end": 260330090}, {"filename": "/GameData/textures/gloves.png", "start": 260330090, "end": 260439765}, {"filename": "/GameData/textures/jacket.png", "start": 260439765, "end": 260603577}, {"filename": "/GameData/textures/levelPlans/1.png", "start": 260603577, "end": 266240691}, {"filename": "/GameData/textures/levelPlans/1_2.png", "start": 266240691, "end": 269640345}, {"filename": "/GameData/textures/levelPlans/lvl1.jpg", "start": 269640345, "end": 273395096}, {"filename": "/GameData/textures/lq_conc/conc1_1.png", "start": 273395096, "end": 273408209}, {"filename": "/GameData/textures/lq_conc/conc1_10.png", "start": 273408209, "end": 273422323}, {"filename": "/GameData/textures/lq_conc/conc1_2.png", "start": 273422323, "end": 273436364}, {"filename": "/GameData/textures/lq_conc/conc1_3.png", "start": 273436364, "end": 273450874}, {"filename": "/GameData/textures/lq_conc/conc1_4.png", "start": 273450874, "end": 273465260}, {"filename": "/GameData/textures/lq_conc/conc1_5.png", "start": 273465260, "end": 273479709}, {"filename": "/GameData/textures/lq_conc/conc1_6.png", "start": 273479709, "end": 273494326}, {"filename": "/GameData/textures/lq_conc/conc1_7.png", "start": 273494326, "end": 273508923}, {"filename": "/GameData/textures/lq_conc/conc1_8.png", "start": 273508923, "end": 273523271}, {"filename": "/GameData/textures/lq_conc/conc1_9.png", "start": 273523271, "end": 273537429}, {"filename": "/GameData/textures/lq_conc/conc1_a1.png", "start": 273537429, "end": 273551877}, {"filename": "/GameData/textures/lq_conc/conc2_1.png", "start": 273551877, "end": 273561305}, {"filename": "/GameData/textures/lq_conc/conc2_10.png", "start": 273561305, "end": 273570754}, {"filename": "/GameData/textures/lq_conc/conc2_2.png", "start": 273570754, "end": 273580148}, {"filename": "/GameData/textures/lq_conc/conc2_3.png", "start": 273580148, "end": 273589793}, {"filename": "/GameData/textures/lq_conc/conc2_4.png", "start": 273589793, "end": 273599300}, {"filename": "/GameData/textures/lq_conc/conc2_5.png", "start": 273599300, "end": 273608805}, {"filename": "/GameData/textures/lq_conc/conc2_6.png", "start": 273608805, "end": 273618318}, {"filename": "/GameData/textures/lq_conc/conc2_7.png", "start": 273618318, "end": 273627863}, {"filename": "/GameData/textures/lq_conc/conc2_8.png", "start": 273627863, "end": 273637273}, {"filename": "/GameData/textures/lq_conc/conc2_9.png", "start": 273637273, "end": 273646810}, {"filename": "/GameData/textures/lq_conc/conc2_a1.png", "start": 273646810, "end": 273655840}, {"filename": "/GameData/textures/lq_conc/conc3_1.png", "start": 273655840, "end": 273663704}, {"filename": "/GameData/textures/lq_conc/conc3_10.png", "start": 273663704, "end": 273672640}, {"filename": "/GameData/textures/lq_conc/conc3_2.png", "start": 273672640, "end": 273681034}, {"filename": "/GameData/textures/lq_conc/conc3_3.png", "start": 273681034, "end": 273691261}, {"filename": "/GameData/textures/lq_conc/conc3_4.png", "start": 273691261, "end": 273700540}, {"filename": "/GameData/textures/lq_conc/conc3_5.png", "start": 273700540, "end": 273710270}, {"filename": "/GameData/textures/lq_conc/conc3_6.png", "start": 273710270, "end": 273719830}, {"filename": "/GameData/textures/lq_conc/conc3_7.png", "start": 273719830, "end": 273728655}, {"filename": "/GameData/textures/lq_conc/conc3_8.png", "start": 273728655, "end": 273737436}, {"filename": "/GameData/textures/lq_conc/conc3_9.png", "start": 273737436, "end": 273746912}, {"filename": "/GameData/textures/lq_conc/conc3_a1.png", "start": 273746912, "end": 273755319}, {"filename": "/GameData/textures/lq_conc/conc4_1.png", "start": 273755319, "end": 273761478}, {"filename": "/GameData/textures/lq_conc/conc4_10.png", "start": 273761478, "end": 273767838}, {"filename": "/GameData/textures/lq_conc/conc4_2.png", "start": 273767838, "end": 273774103}, {"filename": "/GameData/textures/lq_conc/conc4_3.png", "start": 273774103, "end": 273781029}, {"filename": "/GameData/textures/lq_conc/conc4_4.png", "start": 273781029, "end": 273787519}, {"filename": "/GameData/textures/lq_conc/conc4_5.png", "start": 273787519, "end": 273794112}, {"filename": "/GameData/textures/lq_conc/conc4_6.png", "start": 273794112, "end": 273800636}, {"filename": "/GameData/textures/lq_conc/conc4_7.png", "start": 273800636, "end": 273807044}, {"filename": "/GameData/textures/lq_conc/conc4_8.png", "start": 273807044, "end": 273813327}, {"filename": "/GameData/textures/lq_conc/conc4_9.png", "start": 273813327, "end": 273819910}, {"filename": "/GameData/textures/lq_conc/conc4_a1.png", "start": 273819910, "end": 273825979}, {"filename": "/GameData/textures/lq_conc/conc5_1.png", "start": 273825979, "end": 273834741}, {"filename": "/GameData/textures/lq_conc/conc5_10.png", "start": 273834741, "end": 273844356}, {"filename": "/GameData/textures/lq_conc/conc5_2.png", "start": 273844356, "end": 273853672}, {"filename": "/GameData/textures/lq_conc/conc5_8.png", "start": 273853672, "end": 273862752}, {"filename": "/GameData/textures/lq_conc/conc5_9.png", "start": 273862752, "end": 273872466}, {"filename": "/GameData/textures/lq_conc/conc5_a1.png", "start": 273872466, "end": 273880841}, {"filename": "/GameData/textures/lq_conc/conc6_1.png", "start": 273880841, "end": 273887254}, {"filename": "/GameData/textures/lq_conc/conc6_10.png", "start": 273887254, "end": 273893663}, {"filename": "/GameData/textures/lq_conc/conc6_2.png", "start": 273893663, "end": 273899991}, {"filename": "/GameData/textures/lq_conc/conc6_3.png", "start": 273899991, "end": 273906852}, {"filename": "/GameData/textures/lq_conc/conc6_4.png", "start": 273906852, "end": 273913460}, {"filename": "/GameData/textures/lq_conc/conc6_5.png", "start": 273913460, "end": 273920136}, {"filename": "/GameData/textures/lq_conc/conc6_6.png", "start": 273920136, "end": 273926910}, {"filename": "/GameData/textures/lq_conc/conc6_7.png", "start": 273926910, "end": 273933613}, {"filename": "/GameData/textures/lq_conc/conc6_8.png", "start": 273933613, "end": 273940109}, {"filename": "/GameData/textures/lq_conc/conc6_9.png", "start": 273940109, "end": 273946731}, {"filename": "/GameData/textures/lq_conc/conc6_a1.png", "start": 273946731, "end": 273953375}, {"filename": "/GameData/textures/lq_conc/conc7_1.png", "start": 273953375, "end": 273961165}, {"filename": "/GameData/textures/lq_conc/conc7_10.png", "start": 273961165, "end": 273968843}, {"filename": "/GameData/textures/lq_conc/conc7_2.png", "start": 273968843, "end": 273976469}, {"filename": "/GameData/textures/lq_conc/conc7_3.png", "start": 273976469, "end": 273984413}, {"filename": "/GameData/textures/lq_conc/conc7_4.png", "start": 273984413, "end": 273992181}, {"filename": "/GameData/textures/lq_conc/conc7_5.png", "start": 273992181, "end": 274000005}, {"filename": "/GameData/textures/lq_conc/conc7_6.png", "start": 274000005, "end": 274008004}, {"filename": "/GameData/textures/lq_conc/conc7_7.png", "start": 274008004, "end": 274015984}, {"filename": "/GameData/textures/lq_conc/conc7_8.png", "start": 274015984, "end": 274023808}, {"filename": "/GameData/textures/lq_conc/conc7_9.png", "start": 274023808, "end": 274031608}, {"filename": "/GameData/textures/lq_conc/conc7_a1.png", "start": 274031608, "end": 274039341}, {"filename": "/GameData/textures/lq_conc/flr1_1.png", "start": 274039341, "end": 274041673}, {"filename": "/GameData/textures/lq_conc/flr1_2.png", "start": 274041673, "end": 274044937}, {"filename": "/GameData/textures/lq_conc/flr1_3.png", "start": 274044937, "end": 274048477}, {"filename": "/GameData/textures/lq_conc/flr1_4.png", "start": 274048477, "end": 274051254}, {"filename": "/GameData/textures/lq_conc/flr1_5.png", "start": 274051254, "end": 274054850}, {"filename": "/GameData/textures/lq_conc/flr1_6.png", "start": 274054850, "end": 274057906}, {"filename": "/GameData/textures/lq_conc/flr2_1.png", "start": 274057906, "end": 274060016}, {"filename": "/GameData/textures/lq_conc/flr2_2.png", "start": 274060016, "end": 274063229}, {"filename": "/GameData/textures/lq_conc/flr2_3.png", "start": 274063229, "end": 274066849}, {"filename": "/GameData/textures/lq_conc/flr2_4.png", "start": 274066849, "end": 274070247}, {"filename": "/GameData/textures/lq_conc/flr2_5.png", "start": 274070247, "end": 274073607}, {"filename": "/GameData/textures/lq_conc/flr2_6.png", "start": 274073607, "end": 274076806}, {"filename": "/GameData/textures/lq_conc/flr2_7.png", "start": 274076806, "end": 274079941}, {"filename": "/GameData/textures/lq_conc/flr2_8.png", "start": 274079941, "end": 274082882}, {"filename": "/GameData/textures/lq_dev/clip.png", "start": 274082882, "end": 274083388}, {"filename": "/GameData/textures/lq_dev/dot_blue_a.png", "start": 274083388, "end": 274083944}, {"filename": "/GameData/textures/lq_dev/dot_blue_b.png", "start": 274083944, "end": 274084499}, {"filename": "/GameData/textures/lq_dev/dot_blue_c.png", "start": 274084499, "end": 274085054}, {"filename": "/GameData/textures/lq_dev/dot_brown_a.png", "start": 274085054, "end": 274085609}, {"filename": "/GameData/textures/lq_dev/dot_brown_b.png", "start": 274085609, "end": 274086164}, {"filename": "/GameData/textures/lq_dev/dot_brown_c.png", "start": 274086164, "end": 274086719}, {"filename": "/GameData/textures/lq_dev/dot_green_a.png", "start": 274086719, "end": 274087275}, {"filename": "/GameData/textures/lq_dev/dot_green_b.png", "start": 274087275, "end": 274087830}, {"filename": "/GameData/textures/lq_dev/dot_green_c.png", "start": 274087830, "end": 274088385}, {"filename": "/GameData/textures/lq_dev/dot_grey_a.png", "start": 274088385, "end": 274088938}, {"filename": "/GameData/textures/lq_dev/dot_grey_b.png", "start": 274088938, "end": 274089494}, {"filename": "/GameData/textures/lq_dev/dot_grey_c.png", "start": 274089494, "end": 274090049}, {"filename": "/GameData/textures/lq_dev/dot_olive_a.png", "start": 274090049, "end": 274090604}, {"filename": "/GameData/textures/lq_dev/dot_olive_b.png", "start": 274090604, "end": 274091158}, {"filename": "/GameData/textures/lq_dev/dot_olive_c.png", "start": 274091158, "end": 274091713}, {"filename": "/GameData/textures/lq_dev/dot_orange_a.png", "start": 274091713, "end": 274092269}, {"filename": "/GameData/textures/lq_dev/dot_orange_b.png", "start": 274092269, "end": 274092824}, {"filename": "/GameData/textures/lq_dev/dot_orange_c.png", "start": 274092824, "end": 274093379}, {"filename": "/GameData/textures/lq_dev/dot_pink_a.png", "start": 274093379, "end": 274093935}, {"filename": "/GameData/textures/lq_dev/dot_pink_b.png", "start": 274093935, "end": 274094491}, {"filename": "/GameData/textures/lq_dev/dot_pink_c.png", "start": 274094491, "end": 274095046}, {"filename": "/GameData/textures/lq_dev/dot_purple_a.png", "start": 274095046, "end": 274095602}, {"filename": "/GameData/textures/lq_dev/dot_purple_b.png", "start": 274095602, "end": 274096157}, {"filename": "/GameData/textures/lq_dev/dot_purple_c.png", "start": 274096157, "end": 274096712}, {"filename": "/GameData/textures/lq_dev/dot_red_a.png", "start": 274096712, "end": 274097263}, {"filename": "/GameData/textures/lq_dev/dot_red_b.png", "start": 274097263, "end": 274097814}, {"filename": "/GameData/textures/lq_dev/dot_red_c.png", "start": 274097814, "end": 274098365}, {"filename": "/GameData/textures/lq_dev/dot_tan_a.png", "start": 274098365, "end": 274098921}, {"filename": "/GameData/textures/lq_dev/dot_tan_b.png", "start": 274098921, "end": 274099477}, {"filename": "/GameData/textures/lq_dev/dot_tan_c.png", "start": 274099477, "end": 274100032}, {"filename": "/GameData/textures/lq_dev/dot_yellow_a.png", "start": 274100032, "end": 274100588}, {"filename": "/GameData/textures/lq_dev/dot_yellow_b.png", "start": 274100588, "end": 274101144}, {"filename": "/GameData/textures/lq_dev/dot_yellow_c.png", "start": 274101144, "end": 274101698}, {"filename": "/GameData/textures/lq_dev/floor_blue_a.png", "start": 274101698, "end": 274102515}, {"filename": "/GameData/textures/lq_dev/floor_blue_b.png", "start": 274102515, "end": 274103333}, {"filename": "/GameData/textures/lq_dev/floor_blue_c.png", "start": 274103333, "end": 274104151}, {"filename": "/GameData/textures/lq_dev/floor_brown_a.png", "start": 274104151, "end": 274104973}, {"filename": "/GameData/textures/lq_dev/floor_brown_b.png", "start": 274104973, "end": 274105796}, {"filename": "/GameData/textures/lq_dev/floor_brown_c.png", "start": 274105796, "end": 274106618}, {"filename": "/GameData/textures/lq_dev/floor_green_a.png", "start": 274106618, "end": 274107439}, {"filename": "/GameData/textures/lq_dev/floor_green_b.png", "start": 274107439, "end": 274108261}, {"filename": "/GameData/textures/lq_dev/floor_green_c.png", "start": 274108261, "end": 274109083}, {"filename": "/GameData/textures/lq_dev/floor_grey_a.png", "start": 274109083, "end": 274109893}, {"filename": "/GameData/textures/lq_dev/floor_grey_b.png", "start": 274109893, "end": 274110706}, {"filename": "/GameData/textures/lq_dev/floor_grey_c.png", "start": 274110706, "end": 274111520}, {"filename": "/GameData/textures/lq_dev/floor_olive_a.png", "start": 274111520, "end": 274112338}, {"filename": "/GameData/textures/lq_dev/floor_olive_b.png", "start": 274112338, "end": 274113153}, {"filename": "/GameData/textures/lq_dev/floor_olive_c.png", "start": 274113153, "end": 274113970}, {"filename": "/GameData/textures/lq_dev/floor_orange_a.png", "start": 274113970, "end": 274114793}, {"filename": "/GameData/textures/lq_dev/floor_orange_b.png", "start": 274114793, "end": 274115616}, {"filename": "/GameData/textures/lq_dev/floor_orange_c.png", "start": 274115616, "end": 274116436}, {"filename": "/GameData/textures/lq_dev/floor_pink_a.png", "start": 274116436, "end": 274117259}, {"filename": "/GameData/textures/lq_dev/floor_pink_b.png", "start": 274117259, "end": 274118081}, {"filename": "/GameData/textures/lq_dev/floor_pink_c.png", "start": 274118081, "end": 274118903}, {"filename": "/GameData/textures/lq_dev/floor_purple_a.png", "start": 274118903, "end": 274119725}, {"filename": "/GameData/textures/lq_dev/floor_purple_b.png", "start": 274119725, "end": 274120547}, {"filename": "/GameData/textures/lq_dev/floor_purple_c.png", "start": 274120547, "end": 274121368}, {"filename": "/GameData/textures/lq_dev/floor_red_a.png", "start": 274121368, "end": 274122178}, {"filename": "/GameData/textures/lq_dev/floor_red_b.png", "start": 274122178, "end": 274122988}, {"filename": "/GameData/textures/lq_dev/floor_red_c.png", "start": 274122988, "end": 274123799}, {"filename": "/GameData/textures/lq_dev/floor_tan_a.png", "start": 274123799, "end": 274124619}, {"filename": "/GameData/textures/lq_dev/floor_tan_b.png", "start": 274124619, "end": 274125443}, {"filename": "/GameData/textures/lq_dev/floor_tan_c.png", "start": 274125443, "end": 274126264}, {"filename": "/GameData/textures/lq_dev/floor_yellow_a.png", "start": 274126264, "end": 274127085}, {"filename": "/GameData/textures/lq_dev/floor_yellow_b.png", "start": 274127085, "end": 274127907}, {"filename": "/GameData/textures/lq_dev/floor_yellow_c.png", "start": 274127907, "end": 274128726}, {"filename": "/GameData/textures/lq_dev/hint.png", "start": 274128726, "end": 274129592}, {"filename": "/GameData/textures/lq_dev/hintskip.png", "start": 274129592, "end": 274130485}, {"filename": "/GameData/textures/lq_dev/key_gold_1.png", "start": 274130485, "end": 274131240}, {"filename": "/GameData/textures/lq_dev/key_silver_1.png", "start": 274131240, "end": 274131994}, {"filename": "/GameData/textures/lq_dev/light_fbr.png", "start": 274131994, "end": 274132910}, {"filename": "/GameData/textures/lq_dev/origin.png", "start": 274132910, "end": 274133393}, {"filename": "/GameData/textures/lq_dev/plus_0_button_fbr.png", "start": 274133393, "end": 274133592}, {"filename": "/GameData/textures/lq_dev/plus_0_shoot_fbr.png", "start": 274133592, "end": 274133871}, {"filename": "/GameData/textures/lq_dev/plus_1_button_fbr.png", "start": 274133871, "end": 274134072}, {"filename": "/GameData/textures/lq_dev/plus_1_shoot_fbr.png", "start": 274134072, "end": 274134353}, {"filename": "/GameData/textures/lq_dev/plus_a_button_fbr.png", "start": 274134353, "end": 274134554}, {"filename": "/GameData/textures/lq_dev/plus_a_shoot_fbr.png", "start": 274134554, "end": 274134841}, {"filename": "/GameData/textures/lq_dev/skip.png", "start": 274134841, "end": 274135329}, {"filename": "/GameData/textures/lq_dev/sky_dev.png", "start": 274135329, "end": 274137301}, {"filename": "/GameData/textures/lq_dev/sky_dev.png.bak", "start": 274137301, "end": 274149655}, {"filename": "/GameData/textures/lq_dev/sky_dev_day_fbr.png", "start": 274149655, "end": 274153481}, {"filename": "/GameData/textures/lq_dev/sky_dev_void.png", "start": 274153481, "end": 274154392}, {"filename": "/GameData/textures/lq_dev/star_blood1.png", "start": 274154392, "end": 274154754}, {"filename": "/GameData/textures/lq_dev/star_lava1.png", "start": 274154754, "end": 274155125}, {"filename": "/GameData/textures/lq_dev/star_lavaskip.png", "start": 274155125, "end": 274156238}, {"filename": "/GameData/textures/lq_dev/star_slime1.png", "start": 274156238, "end": 274156608}, {"filename": "/GameData/textures/lq_dev/star_slimeskip.png", "start": 274156608, "end": 274157681}, {"filename": "/GameData/textures/lq_dev/star_smile.png", "start": 274157681, "end": 274158161}, {"filename": "/GameData/textures/lq_dev/star_teleport.png", "start": 274158161, "end": 274158485}, {"filename": "/GameData/textures/lq_dev/star_water1.png", "start": 274158485, "end": 274158857}, {"filename": "/GameData/textures/lq_dev/star_water2.png", "start": 274158857, "end": 274159226}, {"filename": "/GameData/textures/lq_dev/star_waterskip.png", "start": 274159226, "end": 274160966}, {"filename": "/GameData/textures/lq_dev/trigger.png", "start": 274160966, "end": 274161463}, {"filename": "/GameData/textures/lq_dev/wall_blue_a.png", "start": 274161463, "end": 274162143}, {"filename": "/GameData/textures/lq_dev/wall_blue_b.png", "start": 274162143, "end": 274162823}, {"filename": "/GameData/textures/lq_dev/wall_blue_c.png", "start": 274162823, "end": 274163503}, {"filename": "/GameData/textures/lq_dev/wall_brown_a.png", "start": 274163503, "end": 274164185}, {"filename": "/GameData/textures/lq_dev/wall_brown_b.png", "start": 274164185, "end": 274164867}, {"filename": "/GameData/textures/lq_dev/wall_brown_c.png", "start": 274164867, "end": 274165549}, {"filename": "/GameData/textures/lq_dev/wall_green_a.png", "start": 274165549, "end": 274166231}, {"filename": "/GameData/textures/lq_dev/wall_green_b.png", "start": 274166231, "end": 274166913}, {"filename": "/GameData/textures/lq_dev/wall_green_c.png", "start": 274166913, "end": 274167595}, {"filename": "/GameData/textures/lq_dev/wall_grey_a.png", "start": 274167595, "end": 274168269}, {"filename": "/GameData/textures/lq_dev/wall_grey_b.png", "start": 274168269, "end": 274168945}, {"filename": "/GameData/textures/lq_dev/wall_grey_c.png", "start": 274168945, "end": 274169622}, {"filename": "/GameData/textures/lq_dev/wall_olive_a.png", "start": 274169622, "end": 274170302}, {"filename": "/GameData/textures/lq_dev/wall_olive_b.png", "start": 274170302, "end": 274170979}, {"filename": "/GameData/textures/lq_dev/wall_olive_c.png", "start": 274170979, "end": 274171658}, {"filename": "/GameData/textures/lq_dev/wall_orange_a.png", "start": 274171658, "end": 274172340}, {"filename": "/GameData/textures/lq_dev/wall_orange_b.png", "start": 274172340, "end": 274173022}, {"filename": "/GameData/textures/lq_dev/wall_orange_c.png", "start": 274173022, "end": 274173703}, {"filename": "/GameData/textures/lq_dev/wall_pink_a.png", "start": 274173703, "end": 274174385}, {"filename": "/GameData/textures/lq_dev/wall_pink_b.png", "start": 274174385, "end": 274175067}, {"filename": "/GameData/textures/lq_dev/wall_pink_c.png", "start": 274175067, "end": 274175749}, {"filename": "/GameData/textures/lq_dev/wall_purple_a.png", "start": 274175749, "end": 274176432}, {"filename": "/GameData/textures/lq_dev/wall_purple_b.png", "start": 274176432, "end": 274177114}, {"filename": "/GameData/textures/lq_dev/wall_purple_c.png", "start": 274177114, "end": 274177796}, {"filename": "/GameData/textures/lq_dev/wall_red_a.png", "start": 274177796, "end": 274178469}, {"filename": "/GameData/textures/lq_dev/wall_red_b.png", "start": 274178469, "end": 274179142}, {"filename": "/GameData/textures/lq_dev/wall_red_c.png", "start": 274179142, "end": 274179816}, {"filename": "/GameData/textures/lq_dev/wall_tan_a.png", "start": 274179816, "end": 274180498}, {"filename": "/GameData/textures/lq_dev/wall_tan_b.png", "start": 274180498, "end": 274181180}, {"filename": "/GameData/textures/lq_dev/wall_tan_c.png", "start": 274181180, "end": 274181862}, {"filename": "/GameData/textures/lq_dev/wall_yellow_a.png", "start": 274181862, "end": 274182544}, {"filename": "/GameData/textures/lq_dev/wall_yellow_b.png", "start": 274182544, "end": 274183226}, {"filename": "/GameData/textures/lq_dev/wall_yellow_c.png", "start": 274183226, "end": 274183907}, {"filename": "/GameData/textures/lq_dev/{char_0_fbr.png", "start": 274183907, "end": 274184084}, {"filename": "/GameData/textures/lq_dev/{char_1_fbr.png", "start": 274184084, "end": 274184244}, {"filename": "/GameData/textures/lq_dev/{char_2_fbr.png", "start": 274184244, "end": 274184416}, {"filename": "/GameData/textures/lq_dev/{char_3_fbr.png", "start": 274184416, "end": 274184595}, {"filename": "/GameData/textures/lq_dev/{char_4_fbr.png", "start": 274184595, "end": 274184758}, {"filename": "/GameData/textures/lq_dev/{char_5_fbr.png", "start": 274184758, "end": 274184931}, {"filename": "/GameData/textures/lq_dev/{char_6_fbr.png", "start": 274184931, "end": 274185103}, {"filename": "/GameData/textures/lq_dev/{char_7_fbr.png", "start": 274185103, "end": 274185257}, {"filename": "/GameData/textures/lq_dev/{char_8_fbr.png", "start": 274185257, "end": 274185425}, {"filename": "/GameData/textures/lq_dev/{char_9_fbr.png", "start": 274185425, "end": 274185592}, {"filename": "/GameData/textures/lq_dev/{char_a_fbr.png", "start": 274185592, "end": 274185756}, {"filename": "/GameData/textures/lq_dev/{char_b_fbr.png", "start": 274185756, "end": 274185926}, {"filename": "/GameData/textures/lq_dev/{char_c_fbr.png", "start": 274185926, "end": 274186076}, {"filename": "/GameData/textures/lq_dev/{char_d_fbr.png", "start": 274186076, "end": 274186242}, {"filename": "/GameData/textures/lq_dev/{char_e_fbr.png", "start": 274186242, "end": 274186410}, {"filename": "/GameData/textures/lq_dev/{char_f_fbr.png", "start": 274186410, "end": 274186570}, {"filename": "/GameData/textures/lq_dev/{char_g_fbr.png", "start": 274186570, "end": 274186742}, {"filename": "/GameData/textures/lq_dev/{char_h_fbr.png", "start": 274186742, "end": 274186901}, {"filename": "/GameData/textures/lq_dev/{char_i_fbr.png", "start": 274186901, "end": 274187060}, {"filename": "/GameData/textures/lq_dev/{char_j_fbr.png", "start": 274187060, "end": 274187226}, {"filename": "/GameData/textures/lq_dev/{char_k_fbr.png", "start": 274187226, "end": 274187420}, {"filename": "/GameData/textures/lq_dev/{char_l_fbr.png", "start": 274187420, "end": 274187561}, {"filename": "/GameData/textures/lq_dev/{char_m_fbr.png", "start": 274187561, "end": 274187710}, {"filename": "/GameData/textures/lq_dev/{char_n_fbr.png", "start": 274187710, "end": 274187893}, {"filename": "/GameData/textures/lq_dev/{char_o_fbr.png", "start": 274187893, "end": 274188043}, {"filename": "/GameData/textures/lq_dev/{char_p_fbr.png", "start": 274188043, "end": 274188201}, {"filename": "/GameData/textures/lq_dev/{char_q_fbr.png", "start": 274188201, "end": 274188367}, {"filename": "/GameData/textures/lq_dev/{char_r_fbr.png", "start": 274188367, "end": 274188539}, {"filename": "/GameData/textures/lq_dev/{char_s_fbr.png", "start": 274188539, "end": 274188715}, {"filename": "/GameData/textures/lq_dev/{char_t_fbr.png", "start": 274188715, "end": 274188863}, {"filename": "/GameData/textures/lq_dev/{char_trans_fbr.png", "start": 274188863, "end": 274188986}, {"filename": "/GameData/textures/lq_dev/{char_u_fbr.png", "start": 274188986, "end": 274189132}, {"filename": "/GameData/textures/lq_dev/{char_v_fbr.png", "start": 274189132, "end": 274189300}, {"filename": "/GameData/textures/lq_dev/{char_w_fbr.png", "start": 274189300, "end": 274189449}, {"filename": "/GameData/textures/lq_dev/{char_x_fbr.png", "start": 274189449, "end": 274189631}, {"filename": "/GameData/textures/lq_dev/{char_y_fbr.png", "start": 274189631, "end": 274189800}, {"filename": "/GameData/textures/lq_dev/{char_z_fbr.png", "start": 274189800, "end": 274189975}, {"filename": "/GameData/textures/lq_dev/{charlow_a_fbr.png", "start": 274189975, "end": 274190130}, {"filename": "/GameData/textures/lq_dev/{charlow_b_fbr.png", "start": 274190130, "end": 274190289}, {"filename": "/GameData/textures/lq_dev/{charlow_c_fbr.png", "start": 274190289, "end": 274190443}, {"filename": "/GameData/textures/lq_dev/{charlow_d_fbr.png", "start": 274190443, "end": 274190604}, {"filename": "/GameData/textures/lq_dev/{charlow_e_fbr.png", "start": 274190604, "end": 274190758}, {"filename": "/GameData/textures/lq_dev/{charlow_f_fbr.png", "start": 274190758, "end": 274190918}, {"filename": "/GameData/textures/lq_dev/{charlow_g_fbr.png", "start": 274190918, "end": 274191076}, {"filename": "/GameData/textures/lq_dev/{charlow_h_fbr.png", "start": 274191076, "end": 274191230}, {"filename": "/GameData/textures/lq_dev/{charlow_i_fbr.png", "start": 274191230, "end": 274191376}, {"filename": "/GameData/textures/lq_dev/{charlow_j_fbr.png", "start": 274191376, "end": 274191536}, {"filename": "/GameData/textures/lq_dev/{charlow_k_fbr.png", "start": 274191536, "end": 274191715}, {"filename": "/GameData/textures/lq_dev/{charlow_l_fbr.png", "start": 274191715, "end": 274191853}, {"filename": "/GameData/textures/lq_dev/{charlow_m_fbr.png", "start": 274191853, "end": 274192006}, {"filename": "/GameData/textures/lq_dev/{charlow_n_fbr.png", "start": 274192006, "end": 274192156}, {"filename": "/GameData/textures/lq_dev/{charlow_o_fbr.png", "start": 274192156, "end": 274192310}, {"filename": "/GameData/textures/lq_dev/{charlow_p_fbr.png", "start": 274192310, "end": 274192465}, {"filename": "/GameData/textures/lq_dev/{charlow_q_fbr.png", "start": 274192465, "end": 274192615}, {"filename": "/GameData/textures/lq_dev/{charlow_r_fbr.png", "start": 274192615, "end": 274192768}, {"filename": "/GameData/textures/lq_dev/{charlow_s_fbr.png", "start": 274192768, "end": 274192920}, {"filename": "/GameData/textures/lq_dev/{charlow_t_fbr.png", "start": 274192920, "end": 274193086}, {"filename": "/GameData/textures/lq_dev/{charlow_u_fbr.png", "start": 274193086, "end": 274193237}, {"filename": "/GameData/textures/lq_dev/{charlow_v_fbr.png", "start": 274193237, "end": 274193411}, {"filename": "/GameData/textures/lq_dev/{charlow_w_fbr.png", "start": 274193411, "end": 274193565}, {"filename": "/GameData/textures/lq_dev/{charlow_x_fbr.png", "start": 274193565, "end": 274193752}, {"filename": "/GameData/textures/lq_dev/{charlow_y_fbr.png", "start": 274193752, "end": 274193926}, {"filename": "/GameData/textures/lq_dev/{charlow_z_fbr.png", "start": 274193926, "end": 274194097}, {"filename": "/GameData/textures/lq_dev/{chars_add_fbr.png", "start": 274194097, "end": 274194260}, {"filename": "/GameData/textures/lq_dev/{chars_and_fbr.png", "start": 274194260, "end": 274194452}, {"filename": "/GameData/textures/lq_dev/{chars_ardown_fbr.png", "start": 274194452, "end": 274194637}, {"filename": "/GameData/textures/lq_dev/{chars_arleft_fbr.png", "start": 274194637, "end": 274194821}, {"filename": "/GameData/textures/lq_dev/{chars_arright_fbr.png", "start": 274194821, "end": 274195006}, {"filename": "/GameData/textures/lq_dev/{chars_arup_fbr.png", "start": 274195006, "end": 274195184}, {"filename": "/GameData/textures/lq_dev/{chars_at_fbr.png", "start": 274195184, "end": 274195341}, {"filename": "/GameData/textures/lq_dev/{chars_brackc1_fbr.png", "start": 274195341, "end": 274195520}, {"filename": "/GameData/textures/lq_dev/{chars_brackc2_fbr.png", "start": 274195520, "end": 274195704}, {"filename": "/GameData/textures/lq_dev/{chars_brackr1_fbr.png", "start": 274195704, "end": 274195871}, {"filename": "/GameData/textures/lq_dev/{chars_brackr2_fbr.png", "start": 274195871, "end": 274196041}, {"filename": "/GameData/textures/lq_dev/{chars_bracks1_fbr.png", "start": 274196041, "end": 274196195}, {"filename": "/GameData/textures/lq_dev/{chars_bracks2_fbr.png", "start": 274196195, "end": 274196348}, {"filename": "/GameData/textures/lq_dev/{chars_caret_fbr.png", "start": 274196348, "end": 274196528}, {"filename": "/GameData/textures/lq_dev/{chars_colon_fbr.png", "start": 274196528, "end": 274196680}, {"filename": "/GameData/textures/lq_dev/{chars_colonsemi_fbr.png", "start": 274196680, "end": 274196847}, {"filename": "/GameData/textures/lq_dev/{chars_comma_fbr.png", "start": 274196847, "end": 274197003}, {"filename": "/GameData/textures/lq_dev/{chars_div_fbr.png", "start": 274197003, "end": 274197172}, {"filename": "/GameData/textures/lq_dev/{chars_dollar_fbr.png", "start": 274197172, "end": 274197348}, {"filename": "/GameData/textures/lq_dev/{chars_equ_fbr.png", "start": 274197348, "end": 274197507}, {"filename": "/GameData/textures/lq_dev/{chars_exclam_fbr.png", "start": 274197507, "end": 274197651}, {"filename": "/GameData/textures/lq_dev/{chars_grave_fbr.png", "start": 274197651, "end": 274197801}, {"filename": "/GameData/textures/lq_dev/{chars_hash_fbr.png", "start": 274197801, "end": 274197986}, {"filename": "/GameData/textures/lq_dev/{chars_heart_fbr.png", "start": 274197986, "end": 274198171}, {"filename": "/GameData/textures/lq_dev/{chars_multi_fbr.png", "start": 274198171, "end": 274198340}, {"filename": "/GameData/textures/lq_dev/{chars_percent_fbr.png", "start": 274198340, "end": 274198546}, {"filename": "/GameData/textures/lq_dev/{chars_perio_fbr.png", "start": 274198546, "end": 274198683}, {"filename": "/GameData/textures/lq_dev/{chars_pipe_fbr.png", "start": 274198683, "end": 274198830}, {"filename": "/GameData/textures/lq_dev/{chars_quest_fbr.png", "start": 274198830, "end": 274199003}, {"filename": "/GameData/textures/lq_dev/{chars_slaback_fbr.png", "start": 274199003, "end": 274199190}, {"filename": "/GameData/textures/lq_dev/{chars_slafoward_fbr.png", "start": 274199190, "end": 274199369}, {"filename": "/GameData/textures/lq_dev/{chars_smile_fbr.png", "start": 274199369, "end": 274199529}, {"filename": "/GameData/textures/lq_dev/{chars_sub_fbr.png", "start": 274199529, "end": 274199676}, {"filename": "/GameData/textures/lq_dev/{chars_sun_fbr.png", "start": 274199676, "end": 274199875}, {"filename": "/GameData/textures/lq_dev/{chars_thngreater_fbr.png", "start": 274199875, "end": 274200068}, {"filename": "/GameData/textures/lq_dev/{chars_thnless_fbr.png", "start": 274200068, "end": 274200255}, {"filename": "/GameData/textures/lq_dev/{chars_tilde_fbr.png", "start": 274200255, "end": 274200424}, {"filename": "/GameData/textures/lq_dev/{chars_unders_fbr.png", "start": 274200424, "end": 274200561}, {"filename": "/GameData/textures/lq_flesh/bone1_1.png", "start": 274200561, "end": 274207968}, {"filename": "/GameData/textures/lq_flesh/bone1_2.png", "start": 274207968, "end": 274217160}, {"filename": "/GameData/textures/lq_flesh/bone2_1.png", "start": 274217160, "end": 274227592}, {"filename": "/GameData/textures/lq_flesh/dopefish_fbr.png", "start": 274227592, "end": 274235822}, {"filename": "/GameData/textures/lq_flesh/flesh_gut1.png", "start": 274235822, "end": 274246137}, {"filename": "/GameData/textures/lq_flesh/flesh_gut2.png", "start": 274246137, "end": 274257937}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_1.png", "start": 274257937, "end": 274271687}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_2.png", "start": 274271687, "end": 274284667}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_3.png", "start": 274284667, "end": 274296700}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_4a.png", "start": 274296700, "end": 274309913}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_4b.png", "start": 274309913, "end": 274323053}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_5a.png", "start": 274323053, "end": 274336697}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_5b.png", "start": 274336697, "end": 274350270}, {"filename": "/GameData/textures/lq_flesh/flesh_rot2_1.png", "start": 274350270, "end": 274363496}, {"filename": "/GameData/textures/lq_flesh/flesh_rot3_1.png", "start": 274363496, "end": 274378442}, {"filename": "/GameData/textures/lq_flesh/flesh_rot3_2.png", "start": 274378442, "end": 274393481}, {"filename": "/GameData/textures/lq_flesh/flesh_rot4_1.png", "start": 274393481, "end": 274408118}, {"filename": "/GameData/textures/lq_flesh/flesh_rot5_1.png", "start": 274408118, "end": 274420386}, {"filename": "/GameData/textures/lq_flesh/flesh_rot6_1.png", "start": 274420386, "end": 274436793}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_1.png", "start": 274436793, "end": 274448311}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_2.png", "start": 274448311, "end": 274461373}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_3.png", "start": 274461373, "end": 274472376}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_4.png", "start": 274472376, "end": 274483788}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_5.png", "start": 274483788, "end": 274494387}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_1.png", "start": 274494387, "end": 274498043}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_2.png", "start": 274498043, "end": 274502140}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_3.png", "start": 274502140, "end": 274505516}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_4.png", "start": 274505516, "end": 274509114}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_5.png", "start": 274509114, "end": 274512504}, {"filename": "/GameData/textures/lq_flesh/fleshtile.png", "start": 274512504, "end": 274523473}, {"filename": "/GameData/textures/lq_flesh/marbred128.png", "start": 274523473, "end": 274535702}, {"filename": "/GameData/textures/lq_flesh/may_eat_eye2_fbr.png", "start": 274535702, "end": 274539575}, {"filename": "/GameData/textures/lq_flesh/may_eat_eye3_fbr.png", "start": 274539575, "end": 274543452}, {"filename": "/GameData/textures/lq_flesh/may_eat_eye_fbr.png", "start": 274543452, "end": 274547327}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_b.png", "start": 274547327, "end": 274559990}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_c.png", "start": 274559990, "end": 274572714}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_hol1.png", "start": 274572714, "end": 274577386}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_lit1_fbr.png", "start": 274577386, "end": 274581456}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_shut1.png", "start": 274581456, "end": 274596422}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_sp.png", "start": 274596422, "end": 274616564}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_sp2.png", "start": 274616564, "end": 274642187}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_te.png", "start": 274642187, "end": 274657293}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_tet.png", "start": 274657293, "end": 274670513}, {"filename": "/GameData/textures/lq_flesh/may_flesh2_b.png", "start": 274670513, "end": 274684036}, {"filename": "/GameData/textures/lq_flesh/may_flesh3_1a.png", "start": 274684036, "end": 274697762}, {"filename": "/GameData/textures/lq_flesh/may_flesh3_1b.png", "start": 274697762, "end": 274734324}, {"filename": "/GameData/textures/lq_flesh/may_flesh3_b.png", "start": 274734324, "end": 274748210}, {"filename": "/GameData/textures/lq_flesh/may_flesh4_det.png", "start": 274748210, "end": 274758521}, {"filename": "/GameData/textures/lq_flesh/may_flesh4a_det.png", "start": 274758521, "end": 274773382}, {"filename": "/GameData/textures/lq_flesh/may_flesh5.png", "start": 274773382, "end": 274783718}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1a.png", "start": 274783718, "end": 274838453}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1b.png", "start": 274838453, "end": 274891171}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1c.png", "start": 274891171, "end": 274934813}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1lit_fbr.png", "start": 274934813, "end": 274951747}, {"filename": "/GameData/textures/lq_flesh/may_flesh_dr1a.png", "start": 274951747, "end": 275008640}, {"filename": "/GameData/textures/lq_flesh/may_skin_eye.png", "start": 275008640, "end": 275019857}, {"filename": "/GameData/textures/lq_flesh/meat-teeth0.png", "start": 275019857, "end": 275063228}, {"filename": "/GameData/textures/lq_flesh/meat-teeth1.png", "start": 275063228, "end": 275108420}, {"filename": "/GameData/textures/lq_flesh/meat_det1.png", "start": 275108420, "end": 275127151}, {"filename": "/GameData/textures/lq_flesh/meat_det2.png", "start": 275127151, "end": 275149722}, {"filename": "/GameData/textures/lq_flesh/meat_pipe1.png", "start": 275149722, "end": 275174146}, {"filename": "/GameData/textures/lq_flesh/plus_0eye.png", "start": 275174146, "end": 275178291}, {"filename": "/GameData/textures/lq_flesh/plus_0flesh2_gl.png", "start": 275178291, "end": 275193471}, {"filename": "/GameData/textures/lq_flesh/plus_0flesh_but1_fbr.png", "start": 275193471, "end": 275197766}, {"filename": "/GameData/textures/lq_flesh/plus_0flesh_but2_fbr.png", "start": 275197766, "end": 275201955}, {"filename": "/GameData/textures/lq_flesh/plus_0flsh_vent.png", "start": 275201955, "end": 275216206}, {"filename": "/GameData/textures/lq_flesh/plus_1eye.png", "start": 275216206, "end": 275220350}, {"filename": "/GameData/textures/lq_flesh/plus_1flesh2_gl.png", "start": 275220350, "end": 275235509}, {"filename": "/GameData/textures/lq_flesh/plus_1flesh_but2_fbr.png", "start": 275235509, "end": 275239704}, {"filename": "/GameData/textures/lq_flesh/plus_2eye.png", "start": 275239704, "end": 275243744}, {"filename": "/GameData/textures/lq_flesh/plus_2flesh2_gl.png", "start": 275243744, "end": 275258920}, {"filename": "/GameData/textures/lq_flesh/plus_2flesh_but2_fbr.png", "start": 275258920, "end": 275263103}, {"filename": "/GameData/textures/lq_flesh/plus_3eye.png", "start": 275263103, "end": 275267151}, {"filename": "/GameData/textures/lq_flesh/plus_3flesh2_gl.png", "start": 275267151, "end": 275282325}, {"filename": "/GameData/textures/lq_flesh/plus_3flesh_but2_fbr.png", "start": 275282325, "end": 275286496}, {"filename": "/GameData/textures/lq_flesh/plus_4eye.png", "start": 275286496, "end": 275290615}, {"filename": "/GameData/textures/lq_flesh/plus_4flesh2_gl.png", "start": 275290615, "end": 275305795}, {"filename": "/GameData/textures/lq_flesh/plus_5eye.png", "start": 275305795, "end": 275309848}, {"filename": "/GameData/textures/lq_flesh/plus_6eye.png", "start": 275309848, "end": 275313896}, {"filename": "/GameData/textures/lq_flesh/plus_7eye.png", "start": 275313896, "end": 275317967}, {"filename": "/GameData/textures/lq_flesh/plus_8eye.png", "start": 275317967, "end": 275322075}, {"filename": "/GameData/textures/lq_flesh/plus_9eye.png", "start": 275322075, "end": 275326223}, {"filename": "/GameData/textures/lq_flesh/plus_aeye.png", "start": 275326223, "end": 275330215}, {"filename": "/GameData/textures/lq_flesh/plus_aflesh_but1.png", "start": 275330215, "end": 275334781}, {"filename": "/GameData/textures/lq_flesh/plus_aflesh_but2.png", "start": 275334781, "end": 275339310}, {"filename": "/GameData/textures/lq_flesh/plus_aflsh_vent.png", "start": 275339310, "end": 275353710}, {"filename": "/GameData/textures/lq_greek/black.png", "start": 275353710, "end": 275353855}, {"filename": "/GameData/textures/lq_greek/grk_arch1.png", "start": 275353855, "end": 275358872}, {"filename": "/GameData/textures/lq_greek/grk_arch1_2.png", "start": 275358872, "end": 275364498}, {"filename": "/GameData/textures/lq_greek/grk_arch1_a.png", "start": 275364498, "end": 275377273}, {"filename": "/GameData/textures/lq_greek/grk_arch1_b.png", "start": 275377273, "end": 275390733}, {"filename": "/GameData/textures/lq_greek/grk_arch1_c.png", "start": 275390733, "end": 275403906}, {"filename": "/GameData/textures/lq_greek/grk_arch1_d.png", "start": 275403906, "end": 275417423}, {"filename": "/GameData/textures/lq_greek/grk_arch2.png", "start": 275417423, "end": 275420705}, {"filename": "/GameData/textures/lq_greek/grk_arch2_2.png", "start": 275420705, "end": 275424027}, {"filename": "/GameData/textures/lq_greek/grk_arch2_a.png", "start": 275424027, "end": 275434430}, {"filename": "/GameData/textures/lq_greek/grk_arch2_b.png", "start": 275434430, "end": 275444947}, {"filename": "/GameData/textures/lq_greek/grk_arch2_c.png", "start": 275444947, "end": 275455076}, {"filename": "/GameData/textures/lq_greek/grk_arch_end.png", "start": 275455076, "end": 275455832}, {"filename": "/GameData/textures/lq_greek/grk_arch_tcap.png", "start": 275455832, "end": 275456546}, {"filename": "/GameData/textures/lq_greek/grk_arch_trim.png", "start": 275456546, "end": 275458018}, {"filename": "/GameData/textures/lq_greek/grk_bl_arch1a.png", "start": 275458018, "end": 275465653}, {"filename": "/GameData/textures/lq_greek/grk_bl_arch1b.png", "start": 275465653, "end": 275474188}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk1a.png", "start": 275474188, "end": 275476660}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk1b.png", "start": 275476660, "end": 275478798}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk2a.png", "start": 275478798, "end": 275481219}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk2b.png", "start": 275481219, "end": 275483679}, {"filename": "/GameData/textures/lq_greek/grk_bl_flt1.png", "start": 275483679, "end": 275485903}, {"filename": "/GameData/textures/lq_greek/grk_bl_fsh1.png", "start": 275485903, "end": 275488829}, {"filename": "/GameData/textures/lq_greek/grk_bl_fsh2.png", "start": 275488829, "end": 275496921}, {"filename": "/GameData/textures/lq_greek/grk_bl_pil1.png", "start": 275496921, "end": 275499708}, {"filename": "/GameData/textures/lq_greek/grk_bl_trim1.png", "start": 275499708, "end": 275502120}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll1.png", "start": 275502120, "end": 275504840}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll2.png", "start": 275504840, "end": 275507566}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll3a.png", "start": 275507566, "end": 275509924}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll3b.png", "start": 275509924, "end": 275512392}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll4b.png", "start": 275512392, "end": 275514958}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5a.png", "start": 275514958, "end": 275517175}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5b.png", "start": 275517175, "end": 275519474}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5c.png", "start": 275519474, "end": 275521797}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5d.png", "start": 275521797, "end": 275524204}, {"filename": "/GameData/textures/lq_greek/grk_brk15.png", "start": 275524204, "end": 275533710}, {"filename": "/GameData/textures/lq_greek/grk_brk15_b.png", "start": 275533710, "end": 275542958}, {"filename": "/GameData/textures/lq_greek/grk_brk15_c.png", "start": 275542958, "end": 275559826}, {"filename": "/GameData/textures/lq_greek/grk_brk15_f.png", "start": 275559826, "end": 275568119}, {"filename": "/GameData/textures/lq_greek/grk_brk15_g.png", "start": 275568119, "end": 275575768}, {"filename": "/GameData/textures/lq_greek/grk_brk16.png", "start": 275575768, "end": 275585044}, {"filename": "/GameData/textures/lq_greek/grk_brk16_a.png", "start": 275585044, "end": 275594323}, {"filename": "/GameData/textures/lq_greek/grk_brk16_f.png", "start": 275594323, "end": 275604028}, {"filename": "/GameData/textures/lq_greek/grk_brk17.png", "start": 275604028, "end": 275639172}, {"filename": "/GameData/textures/lq_greek/grk_brk17_f.png", "start": 275639172, "end": 275651774}, {"filename": "/GameData/textures/lq_greek/grk_but1.png", "start": 275651774, "end": 275652723}, {"filename": "/GameData/textures/lq_greek/grk_det1.png", "start": 275652723, "end": 275655796}, {"filename": "/GameData/textures/lq_greek/grk_door1.png", "start": 275655796, "end": 275670903}, {"filename": "/GameData/textures/lq_greek/grk_door1_f.png", "start": 275670903, "end": 275680938}, {"filename": "/GameData/textures/lq_greek/grk_door2.png", "start": 275680938, "end": 275691798}, {"filename": "/GameData/textures/lq_greek/grk_door3.png", "start": 275691798, "end": 275704455}, {"filename": "/GameData/textures/lq_greek/grk_ebrick10.png", "start": 275704455, "end": 275715082}, {"filename": "/GameData/textures/lq_greek/grk_ebrick10_bl.png", "start": 275715082, "end": 275729616}, {"filename": "/GameData/textures/lq_greek/grk_ebrick22.png", "start": 275729616, "end": 275743482}, {"filename": "/GameData/textures/lq_greek/grk_ebrick23.png", "start": 275743482, "end": 275752848}, {"filename": "/GameData/textures/lq_greek/grk_ebrick24.png", "start": 275752848, "end": 275761041}, {"filename": "/GameData/textures/lq_greek/grk_flr1.png", "start": 275761041, "end": 275761372}, {"filename": "/GameData/textures/lq_greek/grk_flr2.png", "start": 275761372, "end": 275762226}, {"filename": "/GameData/textures/lq_greek/grk_flr3.png", "start": 275762226, "end": 275764822}, {"filename": "/GameData/textures/lq_greek/grk_flr4_1.png", "start": 275764822, "end": 275766952}, {"filename": "/GameData/textures/lq_greek/grk_flr4_2.png", "start": 275766952, "end": 275769214}, {"filename": "/GameData/textures/lq_greek/grk_flr4_3.png", "start": 275769214, "end": 275771577}, {"filename": "/GameData/textures/lq_greek/grk_flr4_4.png", "start": 275771577, "end": 275773872}, {"filename": "/GameData/textures/lq_greek/grk_flr4_5.png", "start": 275773872, "end": 275777504}, {"filename": "/GameData/textures/lq_greek/grk_flr4_6.png", "start": 275777504, "end": 275780510}, {"filename": "/GameData/textures/lq_greek/grk_flr4_8.png", "start": 275780510, "end": 275783845}, {"filename": "/GameData/textures/lq_greek/grk_flr5_1.png", "start": 275783845, "end": 275786432}, {"filename": "/GameData/textures/lq_greek/grk_flr5_2.png", "start": 275786432, "end": 275788926}, {"filename": "/GameData/textures/lq_greek/grk_flr5_3.png", "start": 275788926, "end": 275791668}, {"filename": "/GameData/textures/lq_greek/grk_flr5_4.png", "start": 275791668, "end": 275793971}, {"filename": "/GameData/textures/lq_greek/grk_key_gl2.png", "start": 275793971, "end": 275794995}, {"filename": "/GameData/textures/lq_greek/grk_key_sl2.png", "start": 275794995, "end": 275795996}, {"filename": "/GameData/textures/lq_greek/grk_lion1.png", "start": 275795996, "end": 275806470}, {"filename": "/GameData/textures/lq_greek/grk_lion2.png", "start": 275806470, "end": 275816137}, {"filename": "/GameData/textures/lq_greek/grk_lion3.png", "start": 275816137, "end": 275826680}, {"filename": "/GameData/textures/lq_greek/grk_lion4.png", "start": 275826680, "end": 275837832}, {"filename": "/GameData/textures/lq_greek/grk_met1.png", "start": 275837832, "end": 275848213}, {"filename": "/GameData/textures/lq_greek/grk_met1_trim.png", "start": 275848213, "end": 275859488}, {"filename": "/GameData/textures/lq_greek/grk_met1a_trim.png", "start": 275859488, "end": 275870456}, {"filename": "/GameData/textures/lq_greek/grk_met1b_trim.png", "start": 275870456, "end": 275884569}, {"filename": "/GameData/textures/lq_greek/grk_met2_trim.png", "start": 275884569, "end": 275897838}, {"filename": "/GameData/textures/lq_greek/grk_met_plt.png", "start": 275897838, "end": 275913229}, {"filename": "/GameData/textures/lq_greek/grk_mural1.png", "start": 275913229, "end": 275933087}, {"filename": "/GameData/textures/lq_greek/grk_mural2.png", "start": 275933087, "end": 275955804}, {"filename": "/GameData/textures/lq_greek/grk_mural3.png", "start": 275955804, "end": 276045084}, {"filename": "/GameData/textures/lq_greek/grk_pl1_a.png", "start": 276045084, "end": 276046900}, {"filename": "/GameData/textures/lq_greek/grk_pl1_b.png", "start": 276046900, "end": 276048983}, {"filename": "/GameData/textures/lq_greek/grk_pl2_a.png", "start": 276048983, "end": 276051215}, {"filename": "/GameData/textures/lq_greek/grk_pl2_b.png", "start": 276051215, "end": 276053427}, {"filename": "/GameData/textures/lq_greek/grk_plat1_side.png", "start": 276053427, "end": 276054644}, {"filename": "/GameData/textures/lq_greek/grk_plat1_top.png", "start": 276054644, "end": 276058541}, {"filename": "/GameData/textures/lq_greek/grk_tile2_1.png", "start": 276058541, "end": 276060975}, {"filename": "/GameData/textures/lq_greek/grk_tile2_2.png", "start": 276060975, "end": 276063328}, {"filename": "/GameData/textures/lq_greek/grk_trim1.png", "start": 276063328, "end": 276070085}, {"filename": "/GameData/textures/lq_greek/grk_trim1_3.png", "start": 276070085, "end": 276076699}, {"filename": "/GameData/textures/lq_greek/grk_trim1_3_s.png", "start": 276076699, "end": 276078739}, {"filename": "/GameData/textures/lq_greek/grk_trim1_4_s.png", "start": 276078739, "end": 276080815}, {"filename": "/GameData/textures/lq_greek/grk_trim1_5.png", "start": 276080815, "end": 276087751}, {"filename": "/GameData/textures/lq_greek/grk_trim1_5_s.png", "start": 276087751, "end": 276089971}, {"filename": "/GameData/textures/lq_greek/grk_trim1_6_s.png", "start": 276089971, "end": 276092248}, {"filename": "/GameData/textures/lq_greek/grk_trim1_7_s.png", "start": 276092248, "end": 276095590}, {"filename": "/GameData/textures/lq_greek/grk_trim2.png", "start": 276095590, "end": 276097951}, {"filename": "/GameData/textures/lq_greek/grk_wall1.png", "start": 276097951, "end": 276100983}, {"filename": "/GameData/textures/lq_greek/grk_wall2.png", "start": 276100983, "end": 276103654}, {"filename": "/GameData/textures/lq_greek/grk_wall3.png", "start": 276103654, "end": 276109440}, {"filename": "/GameData/textures/lq_greek/grk_wall3b.png", "start": 276109440, "end": 276136474}, {"filename": "/GameData/textures/lq_greek/grk_win1_a.png", "start": 276136474, "end": 276145613}, {"filename": "/GameData/textures/lq_greek/grk_win1_b.png", "start": 276145613, "end": 276154956}, {"filename": "/GameData/textures/lq_greek/plus_0grk_but1_fbr.png", "start": 276154956, "end": 276156147}, {"filename": "/GameData/textures/lq_greek/plus_0grk_hbut_fbr.png", "start": 276156147, "end": 276157170}, {"filename": "/GameData/textures/lq_greek/plus_1grk_but1_fbr.png", "start": 276157170, "end": 276158345}, {"filename": "/GameData/textures/lq_greek/plus_1grk_hbut_fbr.png", "start": 276158345, "end": 276159426}, {"filename": "/GameData/textures/lq_greek/plus_2grk_but1_fbr.png", "start": 276159426, "end": 276160577}, {"filename": "/GameData/textures/lq_greek/plus_2grk_hbut_fbr.png", "start": 276160577, "end": 276161617}, {"filename": "/GameData/textures/lq_greek/plus_3grk_but1_fbr.png", "start": 276161617, "end": 276162770}, {"filename": "/GameData/textures/lq_greek/plus_3grk_hbut_fbr.png", "start": 276162770, "end": 276163812}, {"filename": "/GameData/textures/lq_greek/plus_agrk_but1.png", "start": 276163812, "end": 276164803}, {"filename": "/GameData/textures/lq_greek/plus_agrk_hbut.png", "start": 276164803, "end": 276165781}, {"filename": "/GameData/textures/lq_health_ammo/ammo_bottom.png", "start": 276165781, "end": 276166711}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_b1_fbr.png", "start": 276166711, "end": 276167319}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_b2_fbr.png", "start": 276167319, "end": 276167931}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_b3_fbr.png", "start": 276167931, "end": 276168738}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_s1_fbr.png", "start": 276168738, "end": 276169331}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_s2_fbr.png", "start": 276169331, "end": 276170321}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_s3_fbr.png", "start": 276170321, "end": 276170954}, {"filename": "/GameData/textures/lq_health_ammo/ammo_fl.png", "start": 276170954, "end": 276171513}, {"filename": "/GameData/textures/lq_health_ammo/ammo_fl2.png", "start": 276171513, "end": 276172068}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_b1_fbr.png", "start": 276172068, "end": 276172822}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_b2_fbr.png", "start": 276172822, "end": 276173507}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_b3.png", "start": 276173507, "end": 276174078}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_s1.png", "start": 276174078, "end": 276174649}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_s2_fbr.png", "start": 276174649, "end": 276175304}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_s3_fbr.png", "start": 276175304, "end": 276175841}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_b1_fbr.png", "start": 276175841, "end": 276176556}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_b2_fbr.png", "start": 276176556, "end": 276177182}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_b3_fbr.png", "start": 276177182, "end": 276177640}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_s1_fbr.png", "start": 276177640, "end": 276178254}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_s2_fbr.png", "start": 276178254, "end": 276178603}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_b1_fbr.png", "start": 276178603, "end": 276179262}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_b2_fbr.png", "start": 276179262, "end": 276179890}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_b3.png", "start": 276179890, "end": 276180667}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_s1_fbr.png", "start": 276180667, "end": 276181173}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_s2_fbr.png", "start": 276181173, "end": 276181751}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_s3_fbr.png", "start": 276181751, "end": 276182302}, {"filename": "/GameData/textures/lq_health_ammo/ammobotsmall.png", "start": 276182302, "end": 276182970}, {"filename": "/GameData/textures/lq_health_ammo/ammotop.png", "start": 276182970, "end": 276183715}, {"filename": "/GameData/textures/lq_health_ammo/ammotopsmall.png", "start": 276183715, "end": 276184266}, {"filename": "/GameData/textures/lq_health_ammo/boom.png", "start": 276184266, "end": 276185284}, {"filename": "/GameData/textures/lq_health_ammo/boomammo_bottom.png", "start": 276185284, "end": 276185650}, {"filename": "/GameData/textures/lq_health_ammo/boomammotop.png", "start": 276185650, "end": 276185992}, {"filename": "/GameData/textures/lq_health_ammo/boomsmall.png", "start": 276185992, "end": 276186667}, {"filename": "/GameData/textures/lq_health_ammo/epboxlarge_fbr.png", "start": 276186667, "end": 276187851}, {"filename": "/GameData/textures/lq_health_ammo/epboxsmall_fbr.png", "start": 276187851, "end": 276188748}, {"filename": "/GameData/textures/lq_health_ammo/explob_s2.png", "start": 276188748, "end": 276189244}, {"filename": "/GameData/textures/lq_health_ammo/hp15_side.png", "start": 276189244, "end": 276189829}, {"filename": "/GameData/textures/lq_health_ammo/hp25_top2.png", "start": 276189829, "end": 276190258}, {"filename": "/GameData/textures/lq_health_ammo/hp_bottom.png", "start": 276190258, "end": 276191165}, {"filename": "/GameData/textures/lq_health_ammo/hp_details.png", "start": 276191165, "end": 276191669}, {"filename": "/GameData/textures/lq_health_ammo/nails.png", "start": 276191669, "end": 276192716}, {"filename": "/GameData/textures/lq_health_ammo/nailssmall.png", "start": 276192716, "end": 276193457}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp100-winq_fbr.png", "start": 276193457, "end": 276196057}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp100_side_fbr.png", "start": 276196057, "end": 276196949}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp100_top_fbr.png", "start": 276196949, "end": 276197680}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp15_top_fbr.png", "start": 276197680, "end": 276198410}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp15winq_fbr.png", "start": 276198410, "end": 276201197}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp25-winq_fbr.png", "start": 276201197, "end": 276203751}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp25_side_fbr.png", "start": 276203751, "end": 276204573}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp25_top_fbr.png", "start": 276204573, "end": 276205307}, {"filename": "/GameData/textures/lq_health_ammo/plus_0explob2_s1_fbr.png", "start": 276205307, "end": 276206410}, {"filename": "/GameData/textures/lq_health_ammo/plus_0explob_s1_fbr.png", "start": 276206410, "end": 276207043}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp100-winq_fbr.png", "start": 276207043, "end": 276209648}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp100_side_fbr.png", "start": 276209648, "end": 276210542}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp100_top_fbr.png", "start": 276210542, "end": 276211276}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp15_top_fbr.png", "start": 276211276, "end": 276212010}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp15winq_fbr.png", "start": 276212010, "end": 276214796}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp25-winq_fbr.png", "start": 276214796, "end": 276217360}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp25_side_fbr.png", "start": 276217360, "end": 276218186}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp25_top_fbr.png", "start": 276218186, "end": 276218925}, {"filename": "/GameData/textures/lq_health_ammo/plus_1explob2_s1_fbr.png", "start": 276218925, "end": 276220032}, {"filename": "/GameData/textures/lq_health_ammo/plus_1explob_s1_fbr.png", "start": 276220032, "end": 276220665}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp100-winq_fbr.png", "start": 276220665, "end": 276223269}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp100_side_fbr.png", "start": 276223269, "end": 276224163}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp25-winq_fbr.png", "start": 276224163, "end": 276226721}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp25_side_fbr.png", "start": 276226721, "end": 276227546}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp25_top_fbr.png", "start": 276227546, "end": 276228284}, {"filename": "/GameData/textures/lq_health_ammo/plus_2explob2_s1_fbr.png", "start": 276228284, "end": 276229380}, {"filename": "/GameData/textures/lq_health_ammo/plus_2explob_s1_fbr.png", "start": 276229380, "end": 276230022}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp100-winq_fbr.png", "start": 276230022, "end": 276232632}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp100_side_fbr.png", "start": 276232632, "end": 276233529}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp25-winq_fbr.png", "start": 276233529, "end": 276236098}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp25_side_fbr.png", "start": 276236098, "end": 276236929}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp25_top_fbr.png", "start": 276236929, "end": 276237671}, {"filename": "/GameData/textures/lq_health_ammo/plus_3explob2_s1_fbr.png", "start": 276237671, "end": 276238767}, {"filename": "/GameData/textures/lq_health_ammo/plus_3explob_s1_fbr.png", "start": 276238767, "end": 276239409}, {"filename": "/GameData/textures/lq_health_ammo/shells.png", "start": 276239409, "end": 276240355}, {"filename": "/GameData/textures/lq_health_ammo/shellssmall.png", "start": 276240355, "end": 276241081}, {"filename": "/GameData/textures/lq_health_ammo/zap.png", "start": 276241081, "end": 276242074}, {"filename": "/GameData/textures/lq_health_ammo/zapsmall.png", "start": 276242074, "end": 276242838}, {"filename": "/GameData/textures/lq_legacy/brick7.png", "start": 276242838, "end": 276245771}, {"filename": "/GameData/textures/lq_legacy/brick8.png", "start": 276245771, "end": 276248858}, {"filename": "/GameData/textures/lq_legacy/button_0.png", "start": 276248858, "end": 276252330}, {"filename": "/GameData/textures/lq_legacy/button_0_grey.png", "start": 276252330, "end": 276255540}, {"filename": "/GameData/textures/lq_legacy/button_0_grn.png", "start": 276255540, "end": 276258875}, {"filename": "/GameData/textures/lq_legacy/button_1.png", "start": 276258875, "end": 276262330}, {"filename": "/GameData/textures/lq_legacy/button_1_grey.png", "start": 276262330, "end": 276265525}, {"filename": "/GameData/textures/lq_legacy/button_1_grn.png", "start": 276265525, "end": 276268826}, {"filename": "/GameData/textures/lq_legacy/floor_temp.png", "start": 276268826, "end": 276270884}, {"filename": "/GameData/textures/lq_legacy/flr.png", "start": 276270884, "end": 276273641}, {"filename": "/GameData/textures/lq_legacy/gardgrass_1.png", "start": 276273641, "end": 276293811}, {"filename": "/GameData/textures/lq_legacy/go-savgx.png", "start": 276293811, "end": 276295560}, {"filename": "/GameData/textures/lq_legacy/grass.png", "start": 276295560, "end": 276298961}, {"filename": "/GameData/textures/lq_legacy/grk_brk15_c_old.png", "start": 276298961, "end": 276309488}, {"filename": "/GameData/textures/lq_legacy/grk_brk17_f_old.png", "start": 276309488, "end": 276318197}, {"filename": "/GameData/textures/lq_legacy/grk_door1_old.png", "start": 276318197, "end": 276329214}, {"filename": "/GameData/textures/lq_legacy/grk_door2_old.png", "start": 276329214, "end": 276338387}, {"filename": "/GameData/textures/lq_legacy/grk_door3_old.png", "start": 276338387, "end": 276347193}, {"filename": "/GameData/textures/lq_legacy/grk_ebrick22_old.png", "start": 276347193, "end": 276356510}, {"filename": "/GameData/textures/lq_legacy/grk_trim1_7_s_old.png", "start": 276356510, "end": 276358715}, {"filename": "/GameData/textures/lq_legacy/ground_1.png", "start": 276358715, "end": 276360896}, {"filename": "/GameData/textures/lq_legacy/leaves.png", "start": 276360896, "end": 276363958}, {"filename": "/GameData/textures/lq_legacy/marble1_4.png", "start": 276363958, "end": 276366861}, {"filename": "/GameData/textures/lq_legacy/marble1_5.png", "start": 276366861, "end": 276369951}, {"filename": "/GameData/textures/lq_legacy/med_cflat1_3.png", "start": 276369951, "end": 276372432}, {"filename": "/GameData/textures/lq_legacy/med_csl_trm3.png", "start": 276372432, "end": 276373757}, {"filename": "/GameData/textures/lq_legacy/med_csl_trm3b.png", "start": 276373757, "end": 276374747}, {"filename": "/GameData/textures/lq_legacy/med_csl_trm3c.png", "start": 276374747, "end": 276376275}, {"filename": "/GameData/textures/lq_legacy/med_flat10.png", "start": 276376275, "end": 276380168}, {"filename": "/GameData/textures/lq_legacy/med_flat11.png", "start": 276380168, "end": 276383373}, {"filename": "/GameData/textures/lq_legacy/med_flat13.png", "start": 276383373, "end": 276394150}, {"filename": "/GameData/textures/lq_legacy/med_flat14.png", "start": 276394150, "end": 276404551}, {"filename": "/GameData/textures/lq_legacy/med_flat9a.png", "start": 276404551, "end": 276448131}, {"filename": "/GameData/textures/lq_legacy/med_flat9b.png", "start": 276448131, "end": 276490030}, {"filename": "/GameData/textures/lq_legacy/med_plaster1.png", "start": 276490030, "end": 276498072}, {"filename": "/GameData/textures/lq_legacy/med_ret_rock1.png", "start": 276498072, "end": 276501891}, {"filename": "/GameData/textures/lq_legacy/med_ret_wood1_old.png", "start": 276501891, "end": 276508972}, {"filename": "/GameData/textures/lq_legacy/med_rmet_key.png", "start": 276508972, "end": 276512933}, {"filename": "/GameData/textures/lq_legacy/med_rock6.png", "start": 276512933, "end": 276558536}, {"filename": "/GameData/textures/lq_legacy/med_rock7.png", "start": 276558536, "end": 276561420}, {"filename": "/GameData/textures/lq_legacy/med_rock8.png", "start": 276561420, "end": 276563305}, {"filename": "/GameData/textures/lq_legacy/med_rough_block.png", "start": 276563305, "end": 276612148}, {"filename": "/GameData/textures/lq_legacy/med_rough_block_f.png", "start": 276612148, "end": 276666693}, {"filename": "/GameData/textures/lq_legacy/med_tile.png", "start": 276666693, "end": 276668829}, {"filename": "/GameData/textures/lq_legacy/med_wall1.png", "start": 276668829, "end": 276678694}, {"filename": "/GameData/textures/lq_legacy/metground_1.png", "start": 276678694, "end": 276681625}, {"filename": "/GameData/textures/lq_legacy/note-savgx.png", "start": 276681625, "end": 276700645}, {"filename": "/GameData/textures/lq_legacy/plus_0button1.png", "start": 276700645, "end": 276704090}, {"filename": "/GameData/textures/lq_legacy/plus_0button2_fbr.png", "start": 276704090, "end": 276705577}, {"filename": "/GameData/textures/lq_legacy/plus_1button2_fbr.png", "start": 276705577, "end": 276707065}, {"filename": "/GameData/textures/lq_legacy/plus_1button3.png", "start": 276707065, "end": 276708945}, {"filename": "/GameData/textures/lq_legacy/plus_abutton1_fbr.png", "start": 276708945, "end": 276712547}, {"filename": "/GameData/textures/lq_legacy/plus_abutton2_fbr.png", "start": 276712547, "end": 276714045}, {"filename": "/GameData/textures/lq_legacy/readme.txt", "start": 276714045, "end": 276714122}, {"filename": "/GameData/textures/lq_legacy/riktoiflat.png", "start": 276714122, "end": 276728241}, {"filename": "/GameData/textures/lq_legacy/riktoiflat_blu.png", "start": 276728241, "end": 276741176}, {"filename": "/GameData/textures/lq_legacy/riktoiflat_grn.png", "start": 276741176, "end": 276752830}, {"filename": "/GameData/textures/lq_legacy/riktoilava.png", "start": 276752830, "end": 276757334}, {"filename": "/GameData/textures/lq_legacy/riktoislime.png", "start": 276757334, "end": 276761039}, {"filename": "/GameData/textures/lq_legacy/riktoitrim.png", "start": 276761039, "end": 276762526}, {"filename": "/GameData/textures/lq_legacy/riktoitrim__purp.png", "start": 276762526, "end": 276764242}, {"filename": "/GameData/textures/lq_legacy/riktoitrim_blu.png", "start": 276764242, "end": 276765972}, {"filename": "/GameData/textures/lq_legacy/riktoiwall.png", "start": 276765972, "end": 276805769}, {"filename": "/GameData/textures/lq_legacy/riktoiwall__purp.png", "start": 276805769, "end": 276841842}, {"filename": "/GameData/textures/lq_legacy/riktoiwall_blu.png", "start": 276841842, "end": 276877903}, {"filename": "/GameData/textures/lq_legacy/riktoiwater.png", "start": 276877903, "end": 276880794}, {"filename": "/GameData/textures/lq_legacy/rune1_fbr.png", "start": 276880794, "end": 276883807}, {"filename": "/GameData/textures/lq_legacy/rune2_fbr.png", "start": 276883807, "end": 276886873}, {"filename": "/GameData/textures/lq_legacy/rune3_fbr.png", "start": 276886873, "end": 276890269}, {"filename": "/GameData/textures/lq_legacy/rune4_fbr.png", "start": 276890269, "end": 276892821}, {"filename": "/GameData/textures/lq_legacy/sign_easy.png", "start": 276892821, "end": 276897773}, {"filename": "/GameData/textures/lq_legacy/sign_empty.png", "start": 276897773, "end": 276902530}, {"filename": "/GameData/textures/lq_legacy/sign_hard.png", "start": 276902530, "end": 276907403}, {"filename": "/GameData/textures/lq_legacy/sign_medium.png", "start": 276907403, "end": 276912413}, {"filename": "/GameData/textures/lq_legacy/sign_metal_1.png", "start": 276912413, "end": 276913625}, {"filename": "/GameData/textures/lq_legacy/sign_metal_2.png", "start": 276913625, "end": 276915220}, {"filename": "/GameData/textures/lq_legacy/sign_nmare.png", "start": 276915220, "end": 276920241}, {"filename": "/GameData/textures/lq_legacy/sky2.png", "start": 276920241, "end": 276935731}, {"filename": "/GameData/textures/lq_legacy/sky4.png", "start": 276935731, "end": 276945243}, {"filename": "/GameData/textures/lq_legacy/sky5_fbr.png", "start": 276945243, "end": 276962396}, {"filename": "/GameData/textures/lq_legacy/sky5a.png", "start": 276962396, "end": 276971407}, {"filename": "/GameData/textures/lq_legacy/sky7.png", "start": 276971407, "end": 276985322}, {"filename": "/GameData/textures/lq_legacy/sky8.png", "start": 276985322, "end": 277001995}, {"filename": "/GameData/textures/lq_legacy/sky8a_fbr.png", "start": 277001995, "end": 277010791}, {"filename": "/GameData/textures/lq_legacy/tile.png", "start": 277010791, "end": 277019607}, {"filename": "/GameData/textures/lq_legacy/tile_blu.png", "start": 277019607, "end": 277028613}, {"filename": "/GameData/textures/lq_legacy/tile_grn.png", "start": 277028613, "end": 277036876}, {"filename": "/GameData/textures/lq_legacy/trim2_blu.png", "start": 277036876, "end": 277037989}, {"filename": "/GameData/textures/lq_legacy/trim2_grn.png", "start": 277037989, "end": 277039053}, {"filename": "/GameData/textures/lq_legacy/ultrasteel1.png", "start": 277039053, "end": 277051943}, {"filename": "/GameData/textures/lq_legacy/ultrasteel2.png", "start": 277051943, "end": 277063737}, {"filename": "/GameData/textures/lq_legacy/vines1_old.png", "start": 277063737, "end": 277068591}, {"filename": "/GameData/textures/lq_legacy/wiz1_4.png", "start": 277068591, "end": 277072412}, {"filename": "/GameData/textures/lq_liquidsky/+0water_f3.png", "start": 277072412, "end": 277075353}, {"filename": "/GameData/textures/lq_liquidsky/+1water_f3.png", "start": 277075353, "end": 277078277}, {"filename": "/GameData/textures/lq_liquidsky/+2water_f3.png", "start": 277078277, "end": 277081195}, {"filename": "/GameData/textures/lq_liquidsky/+3water_f3.png", "start": 277081195, "end": 277084047}, {"filename": "/GameData/textures/lq_liquidsky/plus_0blood_f1.png", "start": 277084047, "end": 277085836}, {"filename": "/GameData/textures/lq_liquidsky/plus_0fslime.png", "start": 277085836, "end": 277099792}, {"filename": "/GameData/textures/lq_liquidsky/plus_0lava_fall3_fbr.png", "start": 277099792, "end": 277108785}, {"filename": "/GameData/textures/lq_liquidsky/plus_0water_f1.png", "start": 277108785, "end": 277110634}, {"filename": "/GameData/textures/lq_liquidsky/plus_0water_f2.png", "start": 277110634, "end": 277112452}, {"filename": "/GameData/textures/lq_liquidsky/plus_0wfall0.png", "start": 277112452, "end": 277122061}, {"filename": "/GameData/textures/lq_liquidsky/plus_1blood_f1.png", "start": 277122061, "end": 277123866}, {"filename": "/GameData/textures/lq_liquidsky/plus_1fslime.png", "start": 277123866, "end": 277137609}, {"filename": "/GameData/textures/lq_liquidsky/plus_1lava_fall3_fbr.png", "start": 277137609, "end": 277147176}, {"filename": "/GameData/textures/lq_liquidsky/plus_1water_f1.png", "start": 277147176, "end": 277148824}, {"filename": "/GameData/textures/lq_liquidsky/plus_1water_f2.png", "start": 277148824, "end": 277150653}, {"filename": "/GameData/textures/lq_liquidsky/plus_1wfall0.png", "start": 277150653, "end": 277160304}, {"filename": "/GameData/textures/lq_liquidsky/plus_2blood_f1.png", "start": 277160304, "end": 277162162}, {"filename": "/GameData/textures/lq_liquidsky/plus_2fslime.png", "start": 277162162, "end": 277176056}, {"filename": "/GameData/textures/lq_liquidsky/plus_2lava_fall3_fbr.png", "start": 277176056, "end": 277185605}, {"filename": "/GameData/textures/lq_liquidsky/plus_2water_f1.png", "start": 277185605, "end": 277187376}, {"filename": "/GameData/textures/lq_liquidsky/plus_2water_f2.png", "start": 277187376, "end": 277189262}, {"filename": "/GameData/textures/lq_liquidsky/plus_2wfall0.png", "start": 277189262, "end": 277198924}, {"filename": "/GameData/textures/lq_liquidsky/plus_3blood_f1.png", "start": 277198924, "end": 277200724}, {"filename": "/GameData/textures/lq_liquidsky/plus_3fslime.png", "start": 277200724, "end": 277214407}, {"filename": "/GameData/textures/lq_liquidsky/plus_3lava_fall3_fbr.png", "start": 277214407, "end": 277223744}, {"filename": "/GameData/textures/lq_liquidsky/plus_3water_f1.png", "start": 277223744, "end": 277225395}, {"filename": "/GameData/textures/lq_liquidsky/plus_3water_f2.png", "start": 277225395, "end": 277227222}, {"filename": "/GameData/textures/lq_liquidsky/plus_3wfall0.png", "start": 277227222, "end": 277236925}, {"filename": "/GameData/textures/lq_liquidsky/plus_4fslime.png", "start": 277236925, "end": 277250589}, {"filename": "/GameData/textures/lq_liquidsky/plus_4lava_fall3_fbr.png", "start": 277250589, "end": 277259962}, {"filename": "/GameData/textures/lq_liquidsky/plus_4wfall0.png", "start": 277259962, "end": 277269541}, {"filename": "/GameData/textures/lq_liquidsky/plus_5fslime.png", "start": 277269541, "end": 277283309}, {"filename": "/GameData/textures/lq_liquidsky/plus_5lava_fall3_fbr.png", "start": 277283309, "end": 277292672}, {"filename": "/GameData/textures/lq_liquidsky/plus_5wfall0.png", "start": 277292672, "end": 277302271}, {"filename": "/GameData/textures/lq_liquidsky/plus_6fslime.png", "start": 277302271, "end": 277316123}, {"filename": "/GameData/textures/lq_liquidsky/plus_6lava_fall3_fbr.png", "start": 277316123, "end": 277325508}, {"filename": "/GameData/textures/lq_liquidsky/plus_6wfall0.png", "start": 277325508, "end": 277335093}, {"filename": "/GameData/textures/lq_liquidsky/plus_7fslime.png", "start": 277335093, "end": 277348944}, {"filename": "/GameData/textures/lq_liquidsky/plus_7lava_fall3_fbr.png", "start": 277348944, "end": 277358459}, {"filename": "/GameData/textures/lq_liquidsky/plus_7wfall0.png", "start": 277358459, "end": 277368128}, {"filename": "/GameData/textures/lq_liquidsky/plus_8wfall0.png", "start": 277368128, "end": 277377790}, {"filename": "/GameData/textures/lq_liquidsky/plus_9wfall0.png", "start": 277377790, "end": 277387412}, {"filename": "/GameData/textures/lq_liquidsky/sky-test.png", "start": 277387412, "end": 277405008}, {"filename": "/GameData/textures/lq_liquidsky/sky-test.xcf", "start": 277405008, "end": 277538273}, {"filename": "/GameData/textures/lq_liquidsky/sky5_blu.png", "start": 277538273, "end": 277551586}, {"filename": "/GameData/textures/lq_liquidsky/sky5_dismal.png", "start": 277551586, "end": 277564769}, {"filename": "/GameData/textures/lq_liquidsky/sky_galx_fbr.png", "start": 277564769, "end": 277587524}, {"filename": "/GameData/textures/lq_liquidsky/sky_galx_spark_fbr.png", "start": 277587524, "end": 277607082}, {"filename": "/GameData/textures/lq_liquidsky/sky_orng.png", "start": 277607082, "end": 277621721}, {"filename": "/GameData/textures/lq_liquidsky/sky_pando.png", "start": 277621721, "end": 277637829}, {"filename": "/GameData/textures/lq_liquidsky/sky_pando2.png", "start": 277637829, "end": 277654364}, {"filename": "/GameData/textures/lq_liquidsky/sky_star.png", "start": 277654364, "end": 277656024}, {"filename": "/GameData/textures/lq_liquidsky/sky_void.png", "start": 277656024, "end": 277656719}, {"filename": "/GameData/textures/lq_liquidsky/sky_wfog_fbr.png", "start": 277656719, "end": 277657625}, {"filename": "/GameData/textures/lq_liquidsky/star_acid.png", "start": 277657625, "end": 277659804}, {"filename": "/GameData/textures/lq_liquidsky/star_blood1.png", "start": 277659804, "end": 277661877}, {"filename": "/GameData/textures/lq_liquidsky/star_lava1_fbr.png", "start": 277661877, "end": 277665082}, {"filename": "/GameData/textures/lq_liquidsky/star_lava2_fbr.png", "start": 277665082, "end": 277669164}, {"filename": "/GameData/textures/lq_liquidsky/star_lava3_fbr.png", "start": 277669164, "end": 277673133}, {"filename": "/GameData/textures/lq_liquidsky/star_lava_void_fbr.png", "start": 277673133, "end": 277676823}, {"filename": "/GameData/textures/lq_liquidsky/star_lavaskip.png", "start": 277676823, "end": 277677936}, {"filename": "/GameData/textures/lq_liquidsky/star_meatgoo2_fbr.png", "start": 277677936, "end": 277681124}, {"filename": "/GameData/textures/lq_liquidsky/star_meatgoo_fbr.png", "start": 277681124, "end": 277683921}, {"filename": "/GameData/textures/lq_liquidsky/star_slime1.png", "start": 277683921, "end": 277690212}, {"filename": "/GameData/textures/lq_liquidsky/star_slime2.png", "start": 277690212, "end": 277693401}, {"filename": "/GameData/textures/lq_liquidsky/star_slime3.png", "start": 277693401, "end": 277695433}, {"filename": "/GameData/textures/lq_liquidsky/star_slime_soul.png", "start": 277695433, "end": 277698223}, {"filename": "/GameData/textures/lq_liquidsky/star_slimeskip.png", "start": 277698223, "end": 277699296}, {"filename": "/GameData/textures/lq_liquidsky/star_soul_drain.png", "start": 277699296, "end": 277702264}, {"filename": "/GameData/textures/lq_liquidsky/star_tele1_fbr.png", "start": 277702264, "end": 277704144}, {"filename": "/GameData/textures/lq_liquidsky/star_tele2_fbr.png", "start": 277704144, "end": 277706945}, {"filename": "/GameData/textures/lq_liquidsky/star_tele3_fbr.png", "start": 277706945, "end": 277709981}, {"filename": "/GameData/textures/lq_liquidsky/star_tele4_fbr.png", "start": 277709981, "end": 277712673}, {"filename": "/GameData/textures/lq_liquidsky/star_water0.png", "start": 277712673, "end": 277715521}, {"filename": "/GameData/textures/lq_liquidsky/star_water1.png", "start": 277715521, "end": 277718630}, {"filename": "/GameData/textures/lq_liquidsky/star_water2.png", "start": 277718630, "end": 277720890}, {"filename": "/GameData/textures/lq_liquidsky/star_water3.png", "start": 277720890, "end": 277723271}, {"filename": "/GameData/textures/lq_liquidsky/star_water4.png", "start": 277723271, "end": 277728058}, {"filename": "/GameData/textures/lq_liquidsky/star_waterskip.png", "start": 277728058, "end": 277729798}, {"filename": "/GameData/textures/lq_liquidsky/star_wstill0.png", "start": 277729798, "end": 277732637}, {"filename": "/GameData/textures/lq_mayan/btn1.png", "start": 277732637, "end": 277735555}, {"filename": "/GameData/textures/lq_mayan/may_arrow.png", "start": 277735555, "end": 277736436}, {"filename": "/GameData/textures/lq_mayan/may_blok1_1.png", "start": 277736436, "end": 277738957}, {"filename": "/GameData/textures/lq_mayan/may_blok1_2.png", "start": 277738957, "end": 277741148}, {"filename": "/GameData/textures/lq_mayan/may_blok1_2_m.png", "start": 277741148, "end": 277744244}, {"filename": "/GameData/textures/lq_mayan/may_blok1_m.png", "start": 277744244, "end": 277753518}, {"filename": "/GameData/textures/lq_mayan/may_blok2_1.png", "start": 277753518, "end": 277756082}, {"filename": "/GameData/textures/lq_mayan/may_blok2_1_m.png", "start": 277756082, "end": 277759303}, {"filename": "/GameData/textures/lq_mayan/may_blok2_2.png", "start": 277759303, "end": 277769566}, {"filename": "/GameData/textures/lq_mayan/may_blok2_2_m.png", "start": 277769566, "end": 277781994}, {"filename": "/GameData/textures/lq_mayan/may_blud1_1.png", "start": 277781994, "end": 277784586}, {"filename": "/GameData/textures/lq_mayan/may_blud1_1m.png", "start": 277784586, "end": 277787806}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_1.png", "start": 277787806, "end": 277790241}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_1m.png", "start": 277790241, "end": 277793311}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_2.png", "start": 277793311, "end": 277795738}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_2m.png", "start": 277795738, "end": 277798766}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_3.png", "start": 277798766, "end": 277801025}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_3m.png", "start": 277801025, "end": 277804434}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_5.png", "start": 277804434, "end": 277807287}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_5m.png", "start": 277807287, "end": 277810936}, {"filename": "/GameData/textures/lq_mayan/may_bnd_skull.png", "start": 277810936, "end": 277813568}, {"filename": "/GameData/textures/lq_mayan/may_brk1_0.png", "start": 277813568, "end": 277816672}, {"filename": "/GameData/textures/lq_mayan/may_brk1_0m.png", "start": 277816672, "end": 277819729}, {"filename": "/GameData/textures/lq_mayan/may_brk1_1.png", "start": 277819729, "end": 277822411}, {"filename": "/GameData/textures/lq_mayan/may_brk1_1m.png", "start": 277822411, "end": 277825610}, {"filename": "/GameData/textures/lq_mayan/may_brk1_2.png", "start": 277825610, "end": 277828561}, {"filename": "/GameData/textures/lq_mayan/may_brk1_2m.png", "start": 277828561, "end": 277831966}, {"filename": "/GameData/textures/lq_mayan/may_brk1_3.png", "start": 277831966, "end": 277834903}, {"filename": "/GameData/textures/lq_mayan/may_brk1_3m.png", "start": 277834903, "end": 277838079}, {"filename": "/GameData/textures/lq_mayan/may_brk2_0.png", "start": 277838079, "end": 277847934}, {"filename": "/GameData/textures/lq_mayan/may_brk2_0_m.png", "start": 277847934, "end": 277860267}, {"filename": "/GameData/textures/lq_mayan/may_brk_old.png", "start": 277860267, "end": 277862866}, {"filename": "/GameData/textures/lq_mayan/may_brk_oldm.png", "start": 277862866, "end": 277866205}, {"filename": "/GameData/textures/lq_mayan/may_deco1_1.png", "start": 277866205, "end": 277877434}, {"filename": "/GameData/textures/lq_mayan/may_deco1_1m.png", "start": 277877434, "end": 277891469}, {"filename": "/GameData/textures/lq_mayan/may_deco1_2.png", "start": 277891469, "end": 277893989}, {"filename": "/GameData/textures/lq_mayan/may_deco1_2my.png", "start": 277893989, "end": 277897377}, {"filename": "/GameData/textures/lq_mayan/may_deco1_3.png", "start": 277897377, "end": 277915950}, {"filename": "/GameData/textures/lq_mayan/may_deco1_3m.png", "start": 277915950, "end": 277936301}, {"filename": "/GameData/textures/lq_mayan/may_door1_1.png", "start": 277936301, "end": 277945768}, {"filename": "/GameData/textures/lq_mayan/may_door1_1m.png", "start": 277945768, "end": 277956859}, {"filename": "/GameData/textures/lq_mayan/may_door2_1.png", "start": 277956859, "end": 277968827}, {"filename": "/GameData/textures/lq_mayan/may_door2_2.png", "start": 277968827, "end": 277980605}, {"filename": "/GameData/textures/lq_mayan/may_drt2_1.png", "start": 277980605, "end": 277983442}, {"filename": "/GameData/textures/lq_mayan/may_flr1_1.png", "start": 277983442, "end": 277986347}, {"filename": "/GameData/textures/lq_mayan/may_flr1_2.png", "start": 277986347, "end": 277989570}, {"filename": "/GameData/textures/lq_mayan/may_flt1_1.png", "start": 277989570, "end": 277991859}, {"filename": "/GameData/textures/lq_mayan/may_flt1_1m.png", "start": 277991859, "end": 277995307}, {"filename": "/GameData/textures/lq_mayan/may_key1_1.png", "start": 277995307, "end": 277996894}, {"filename": "/GameData/textures/lq_mayan/may_key1_2.png", "start": 277996894, "end": 277998891}, {"filename": "/GameData/textures/lq_mayan/may_lite1_1_fbr.png", "start": 277998891, "end": 277999821}, {"filename": "/GameData/textures/lq_mayan/may_lite1_2.png", "start": 277999821, "end": 278000184}, {"filename": "/GameData/textures/lq_mayan/may_lite2_1.png", "start": 278000184, "end": 278003121}, {"filename": "/GameData/textures/lq_mayan/may_lite2_2.png", "start": 278003121, "end": 278003607}, {"filename": "/GameData/textures/lq_mayan/may_lite3_1_fbr.png", "start": 278003607, "end": 278004564}, {"filename": "/GameData/textures/lq_mayan/may_lite_f1.png", "start": 278004564, "end": 278005050}, {"filename": "/GameData/textures/lq_mayan/may_oldmtomb1_1_fbr.png", "start": 278005050, "end": 278018902}, {"filename": "/GameData/textures/lq_mayan/may_oldtomb1_2_fbr.png", "start": 278018902, "end": 278032794}, {"filename": "/GameData/textures/lq_mayan/may_plat_stem_m.png", "start": 278032794, "end": 278033660}, {"filename": "/GameData/textures/lq_mayan/may_plats.png", "start": 278033660, "end": 278036322}, {"filename": "/GameData/textures/lq_mayan/may_platst.png", "start": 278036322, "end": 278038749}, {"filename": "/GameData/textures/lq_mayan/may_platt.png", "start": 278038749, "end": 278041372}, {"filename": "/GameData/textures/lq_mayan/may_plr1_1.png", "start": 278041372, "end": 278044566}, {"filename": "/GameData/textures/lq_mayan/may_tomb1_1_fbr.png", "start": 278044566, "end": 278058418}, {"filename": "/GameData/textures/lq_mayan/may_tomb1_2_fbr.png", "start": 278058418, "end": 278072310}, {"filename": "/GameData/textures/lq_mayan/may_trm1_1.png", "start": 278072310, "end": 278081238}, {"filename": "/GameData/textures/lq_mayan/may_trm1_2.png", "start": 278081238, "end": 278083943}, {"filename": "/GameData/textures/lq_mayan/may_trm1_a.png", "start": 278083943, "end": 278086285}, {"filename": "/GameData/textures/lq_mayan/may_tskull.png", "start": 278086285, "end": 278097291}, {"filename": "/GameData/textures/lq_mayan/may_wall1_1.png", "start": 278097291, "end": 278100273}, {"filename": "/GameData/textures/lq_mayan/may_wall1_2.png", "start": 278100273, "end": 278103239}, {"filename": "/GameData/textures/lq_mayan/may_wall1_3.png", "start": 278103239, "end": 278106433}, {"filename": "/GameData/textures/lq_mayan/may_wall1_3a.png", "start": 278106433, "end": 278109014}, {"filename": "/GameData/textures/lq_mayan/may_wall1_4.png", "start": 278109014, "end": 278112002}, {"filename": "/GameData/textures/lq_mayan/may_wall1_4a.png", "start": 278112002, "end": 278114401}, {"filename": "/GameData/textures/lq_mayan/maya_end_dr1.png", "start": 278114401, "end": 278118787}, {"filename": "/GameData/textures/lq_mayan/maya_end_dr2.png", "start": 278118787, "end": 278123172}, {"filename": "/GameData/textures/lq_mayan/maya_end_trim1.png", "start": 278123172, "end": 278127562}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_btn1.png", "start": 278127562, "end": 278130479}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mpiloilon_fbr.png", "start": 278130479, "end": 278132037}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mpilon_fbr.png", "start": 278132037, "end": 278133612}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mshoohoot_fbr.png", "start": 278133612, "end": 278134557}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mshoot_fbr.png", "start": 278134557, "end": 278135500}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_btn1.png", "start": 278135500, "end": 278138408}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mpiloilon_fbr.png", "start": 278138408, "end": 278139981}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mpilon_fbr.png", "start": 278139981, "end": 278141574}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mshoohoot_fbr.png", "start": 278141574, "end": 278142559}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mshoot_fbr.png", "start": 278142559, "end": 278143543}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_btn1.png", "start": 278143543, "end": 278146434}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_mpilon.png", "start": 278146434, "end": 278148011}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_mshoohoot_fbr.png", "start": 278148011, "end": 278149036}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_mshoot_fbr.png", "start": 278149036, "end": 278150063}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_btn1.png", "start": 278150063, "end": 278153125}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mpiloilon_fbr.png", "start": 278153125, "end": 278154698}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mpilon_fbr.png", "start": 278154698, "end": 278156291}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mshoohoot_fbr.png", "start": 278156291, "end": 278157276}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mshoot_fbr.png", "start": 278157276, "end": 278158260}, {"filename": "/GameData/textures/lq_mayan/plus_4_may_btn1.png", "start": 278158260, "end": 278161323}, {"filename": "/GameData/textures/lq_mayan/plus_5_may_btn1.png", "start": 278161323, "end": 278164064}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_btn1.png", "start": 278164064, "end": 278166787}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_mpiloilon_fbr.png", "start": 278166787, "end": 278168364}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_mpilon_fbr.png", "start": 278168364, "end": 278169948}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_mshoot.png", "start": 278169948, "end": 278170897}, {"filename": "/GameData/textures/lq_medieval/+0med_but1.png", "start": 278170897, "end": 278175152}, {"filename": "/GameData/textures/lq_medieval/+0med_but2.png", "start": 278175152, "end": 278176864}, {"filename": "/GameData/textures/lq_medieval/+0med_but3.png", "start": 278176864, "end": 278178567}, {"filename": "/GameData/textures/lq_medieval/+0med_but_s1.png", "start": 278178567, "end": 278182813}, {"filename": "/GameData/textures/lq_medieval/+0med_sht_but1.png", "start": 278182813, "end": 278184106}, {"filename": "/GameData/textures/lq_medieval/+1med_but3.png", "start": 278184106, "end": 278185808}, {"filename": "/GameData/textures/lq_medieval/+1med_but_s1.png", "start": 278185808, "end": 278190027}, {"filename": "/GameData/textures/lq_medieval/+1med_sht_but1.png", "start": 278190027, "end": 278191340}, {"filename": "/GameData/textures/lq_medieval/+2med_but_s1.png", "start": 278191340, "end": 278195614}, {"filename": "/GameData/textures/lq_medieval/+3med_but_s1.png", "start": 278195614, "end": 278199833}, {"filename": "/GameData/textures/lq_medieval/+amed_but1.png", "start": 278199833, "end": 278203916}, {"filename": "/GameData/textures/lq_medieval/+amed_but2.png", "start": 278203916, "end": 278205634}, {"filename": "/GameData/textures/lq_medieval/+amed_but3.png", "start": 278205634, "end": 278207244}, {"filename": "/GameData/textures/lq_medieval/+amed_but_s1.png", "start": 278207244, "end": 278211457}, {"filename": "/GameData/textures/lq_medieval/+amed_sht_but1.png", "start": 278211457, "end": 278212613}, {"filename": "/GameData/textures/lq_medieval/Art1.png", "start": 278212613, "end": 278345606}, {"filename": "/GameData/textures/lq_medieval/afloor1_4.png", "start": 278345606, "end": 278348607}, {"filename": "/GameData/textures/lq_medieval/afloor1_8.png", "start": 278348607, "end": 278351447}, {"filename": "/GameData/textures/lq_medieval/afloor3_1.png", "start": 278351447, "end": 278354257}, {"filename": "/GameData/textures/lq_medieval/altar1_1.png", "start": 278354257, "end": 278358145}, {"filename": "/GameData/textures/lq_medieval/altar1_3.png", "start": 278358145, "end": 278362626}, {"filename": "/GameData/textures/lq_medieval/altar1_4.png", "start": 278362626, "end": 278366679}, {"filename": "/GameData/textures/lq_medieval/brick0.png", "start": 278366679, "end": 278374901}, {"filename": "/GameData/textures/lq_medieval/brick1.png", "start": 278374901, "end": 278385655}, {"filename": "/GameData/textures/lq_medieval/brick4_s.png", "start": 278385655, "end": 278387588}, {"filename": "/GameData/textures/lq_medieval/brown1.png", "start": 278387588, "end": 278396764}, {"filename": "/GameData/textures/lq_medieval/med_block_1a.png", "start": 278396764, "end": 278443119}, {"filename": "/GameData/textures/lq_medieval/med_block_1b.png", "start": 278443119, "end": 278526483}, {"filename": "/GameData/textures/lq_medieval/med_block_1c.png", "start": 278526483, "end": 278624654}, {"filename": "/GameData/textures/lq_medieval/med_block_1d.png", "start": 278624654, "end": 278733323}, {"filename": "/GameData/textures/lq_medieval/med_block_1e.png", "start": 278733323, "end": 278841816}, {"filename": "/GameData/textures/lq_medieval/med_block_1f.png", "start": 278841816, "end": 278909675}, {"filename": "/GameData/textures/lq_medieval/med_block_1s.png", "start": 278909675, "end": 279027126}, {"filename": "/GameData/textures/lq_medieval/med_block_2a.png", "start": 279027126, "end": 279104510}, {"filename": "/GameData/textures/lq_medieval/med_block_2b.png", "start": 279104510, "end": 279181949}, {"filename": "/GameData/textures/lq_medieval/med_block_2c.png", "start": 279181949, "end": 279270824}, {"filename": "/GameData/textures/lq_medieval/med_block_2d.png", "start": 279270824, "end": 279369018}, {"filename": "/GameData/textures/lq_medieval/med_block_2e.png", "start": 279369018, "end": 279467255}, {"filename": "/GameData/textures/lq_medieval/med_block_2f.png", "start": 279467255, "end": 279531799}, {"filename": "/GameData/textures/lq_medieval/med_block_2s.png", "start": 279531799, "end": 279637551}, {"filename": "/GameData/textures/lq_medieval/med_brk9_ceil1a.png", "start": 279637551, "end": 279649359}, {"filename": "/GameData/textures/lq_medieval/med_brk9_ceil1b.png", "start": 279649359, "end": 279661894}, {"filename": "/GameData/textures/lq_medieval/med_brk9_wal2a.png", "start": 279661894, "end": 279674791}, {"filename": "/GameData/textures/lq_medieval/med_brk9_wal2b.png", "start": 279674791, "end": 279688047}, {"filename": "/GameData/textures/lq_medieval/med_brk9_wal2c.png", "start": 279688047, "end": 279700489}, {"filename": "/GameData/textures/lq_medieval/med_brk9_win1.png", "start": 279700489, "end": 279743487}, {"filename": "/GameData/textures/lq_medieval/med_brk9_win1b.png", "start": 279743487, "end": 279788813}, {"filename": "/GameData/textures/lq_medieval/med_but_side.png", "start": 279788813, "end": 279789811}, {"filename": "/GameData/textures/lq_medieval/med_cmet1.png", "start": 279789811, "end": 279792935}, {"filename": "/GameData/textures/lq_medieval/med_cmet2a.png", "start": 279792935, "end": 279796086}, {"filename": "/GameData/textures/lq_medieval/med_cmet2b.png", "start": 279796086, "end": 279799148}, {"filename": "/GameData/textures/lq_medieval/med_cmet2c.png", "start": 279799148, "end": 279802285}, {"filename": "/GameData/textures/lq_medieval/med_cmet3a.png", "start": 279802285, "end": 279805557}, {"filename": "/GameData/textures/lq_medieval/med_cmet3b.png", "start": 279805557, "end": 279808739}, {"filename": "/GameData/textures/lq_medieval/med_cmet4.png", "start": 279808739, "end": 279811580}, {"filename": "/GameData/textures/lq_medieval/med_cmet5a.png", "start": 279811580, "end": 279814718}, {"filename": "/GameData/textures/lq_medieval/med_cmet5c.png", "start": 279814718, "end": 279817861}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk10.png", "start": 279817861, "end": 279832587}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk10_f.png", "start": 279832587, "end": 279844875}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk10b.png", "start": 279844875, "end": 279859438}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk11.png", "start": 279859438, "end": 279870686}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk12.png", "start": 279870686, "end": 279887635}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk12_f.png", "start": 279887635, "end": 279904968}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk13.png", "start": 279904968, "end": 279961085}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk14.png", "start": 279961085, "end": 279976939}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk14_f.png", "start": 279976939, "end": 279989908}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk14b.png", "start": 279989908, "end": 280003398}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk15.png", "start": 280003398, "end": 280017224}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk15b.png", "start": 280017224, "end": 280031154}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk15f.png", "start": 280031154, "end": 280044352}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk16.png", "start": 280044352, "end": 280060573}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk16b.png", "start": 280060573, "end": 280078165}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk16f.png", "start": 280078165, "end": 280095436}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk17.png", "start": 280095436, "end": 280105543}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk17_f.png", "start": 280105543, "end": 280114568}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk17b.png", "start": 280114568, "end": 280124705}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_f.png", "start": 280124705, "end": 280133338}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_gb.png", "start": 280133338, "end": 280136086}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_gt.png", "start": 280136086, "end": 280138823}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_t.png", "start": 280138823, "end": 280147009}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_tb.png", "start": 280147009, "end": 280157635}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_tc.png", "start": 280157635, "end": 280162164}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18b.png", "start": 280162164, "end": 280170337}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk19_f.png", "start": 280170337, "end": 280182038}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk19_t.png", "start": 280182038, "end": 280193429}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk19b.png", "start": 280193429, "end": 280204749}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk1_1.png", "start": 280204749, "end": 280207234}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk1_2.png", "start": 280207234, "end": 280210405}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk1_3.png", "start": 280210405, "end": 280212648}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk2_1.png", "start": 280212648, "end": 280215197}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk2_2.png", "start": 280215197, "end": 280218570}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk5.png", "start": 280218570, "end": 280221737}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk6_1.png", "start": 280221737, "end": 280224380}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk6_2.png", "start": 280224380, "end": 280229442}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk7_1.png", "start": 280229442, "end": 280232082}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk7_1b.png", "start": 280232082, "end": 280234626}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk7_2.png", "start": 280234626, "end": 280237792}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk8_1c.png", "start": 280237792, "end": 280250397}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk8_1d.png", "start": 280250397, "end": 280263000}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk9_1.png", "start": 280263000, "end": 280276210}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk9_1b.png", "start": 280276210, "end": 280289867}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk9_f.png", "start": 280289867, "end": 280302654}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr1_1.png", "start": 280302654, "end": 280312601}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr2_1.png", "start": 280312601, "end": 280314887}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr2_2.png", "start": 280314887, "end": 280317060}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_1.png", "start": 280317060, "end": 280320800}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_3.png", "start": 280320800, "end": 280324234}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_4.png", "start": 280324234, "end": 280337290}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_5.png", "start": 280337290, "end": 280351808}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr5_1.png", "start": 280351808, "end": 280366738}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr5_2.png", "start": 280366738, "end": 280382266}, {"filename": "/GameData/textures/lq_medieval/med_csl_stp1.png", "start": 280382266, "end": 280384863}, {"filename": "/GameData/textures/lq_medieval/med_csl_stp2.png", "start": 280384863, "end": 280387406}, {"filename": "/GameData/textures/lq_medieval/med_csl_trm1.png", "start": 280387406, "end": 280388731}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1.png", "start": 280388731, "end": 280410204}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t.png", "start": 280410204, "end": 280421591}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t2.png", "start": 280421591, "end": 280434753}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t2b_fbr.png", "start": 280434753, "end": 280448703}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t3.png", "start": 280448703, "end": 280460443}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t4.png", "start": 280460443, "end": 280473219}, {"filename": "/GameData/textures/lq_medieval/med_dbrick2.png", "start": 280473219, "end": 280478753}, {"filename": "/GameData/textures/lq_medieval/med_dbrick3.png", "start": 280478753, "end": 280484127}, {"filename": "/GameData/textures/lq_medieval/med_dbrick4.png", "start": 280484127, "end": 280489435}, {"filename": "/GameData/textures/lq_medieval/med_dbrick5.png", "start": 280489435, "end": 280495257}, {"filename": "/GameData/textures/lq_medieval/med_dbrick6.png", "start": 280495257, "end": 280504418}, {"filename": "/GameData/textures/lq_medieval/med_dbrick6b.png", "start": 280504418, "end": 280513426}, {"filename": "/GameData/textures/lq_medieval/med_dbrick6f.png", "start": 280513426, "end": 280522716}, {"filename": "/GameData/textures/lq_medieval/med_door1.png", "start": 280522716, "end": 280527399}, {"filename": "/GameData/textures/lq_medieval/med_door2.png", "start": 280527399, "end": 280531898}, {"filename": "/GameData/textures/lq_medieval/med_door3.png", "start": 280531898, "end": 280536547}, {"filename": "/GameData/textures/lq_medieval/med_door3b.png", "start": 280536547, "end": 280539088}, {"filename": "/GameData/textures/lq_medieval/med_door4.png", "start": 280539088, "end": 280543652}, {"filename": "/GameData/textures/lq_medieval/med_door4b.png", "start": 280543652, "end": 280546248}, {"filename": "/GameData/textures/lq_medieval/med_dr1a.png", "start": 280546248, "end": 280565032}, {"filename": "/GameData/textures/lq_medieval/med_dr1a_blu.png", "start": 280565032, "end": 280583396}, {"filename": "/GameData/textures/lq_medieval/med_dr1b.png", "start": 280583396, "end": 280607545}, {"filename": "/GameData/textures/lq_medieval/med_dr1b_blu.png", "start": 280607545, "end": 280627164}, {"filename": "/GameData/textures/lq_medieval/med_dr2a.png", "start": 280627164, "end": 280651541}, {"filename": "/GameData/textures/lq_medieval/med_dr2a_blu.png", "start": 280651541, "end": 280671435}, {"filename": "/GameData/textures/lq_medieval/med_dr3a.png", "start": 280671435, "end": 280684918}, {"filename": "/GameData/textures/lq_medieval/med_dr3a_blu.png", "start": 280684918, "end": 280698208}, {"filename": "/GameData/textures/lq_medieval/med_dr3b.png", "start": 280698208, "end": 280712516}, {"filename": "/GameData/textures/lq_medieval/med_dr3b_blu.png", "start": 280712516, "end": 280726722}, {"filename": "/GameData/textures/lq_medieval/med_dr3c.png", "start": 280726722, "end": 280739565}, {"filename": "/GameData/textures/lq_medieval/med_dr3c_blu.png", "start": 280739565, "end": 280752689}, {"filename": "/GameData/textures/lq_medieval/med_dwall1.png", "start": 280752689, "end": 280755165}, {"filename": "/GameData/textures/lq_medieval/med_ebrick1.png", "start": 280755165, "end": 280767747}, {"filename": "/GameData/textures/lq_medieval/med_ebrick10.png", "start": 280767747, "end": 280778632}, {"filename": "/GameData/textures/lq_medieval/med_ebrick10b.png", "start": 280778632, "end": 280789118}, {"filename": "/GameData/textures/lq_medieval/med_ebrick11.png", "start": 280789118, "end": 280804377}, {"filename": "/GameData/textures/lq_medieval/med_ebrick12.png", "start": 280804377, "end": 280821794}, {"filename": "/GameData/textures/lq_medieval/med_ebrick12b.png", "start": 280821794, "end": 280835033}, {"filename": "/GameData/textures/lq_medieval/med_ebrick13.png", "start": 280835033, "end": 280844931}, {"filename": "/GameData/textures/lq_medieval/med_ebrick14.png", "start": 280844931, "end": 280853946}, {"filename": "/GameData/textures/lq_medieval/med_ebrick15.png", "start": 280853946, "end": 280868747}, {"filename": "/GameData/textures/lq_medieval/med_ebrick16.png", "start": 280868747, "end": 280880111}, {"filename": "/GameData/textures/lq_medieval/med_ebrick16b.png", "start": 280880111, "end": 280888429}, {"filename": "/GameData/textures/lq_medieval/med_ebrick17.png", "start": 280888429, "end": 280901563}, {"filename": "/GameData/textures/lq_medieval/med_ebrick17b.png", "start": 280901563, "end": 280913829}, {"filename": "/GameData/textures/lq_medieval/med_ebrick17c.png", "start": 280913829, "end": 280927088}, {"filename": "/GameData/textures/lq_medieval/med_ebrick18.png", "start": 280927088, "end": 280943088}, {"filename": "/GameData/textures/lq_medieval/med_ebrick18b.png", "start": 280943088, "end": 280957476}, {"filename": "/GameData/textures/lq_medieval/med_ebrick2.png", "start": 280957476, "end": 280969968}, {"filename": "/GameData/textures/lq_medieval/med_ebrick20.png", "start": 280969968, "end": 280983368}, {"filename": "/GameData/textures/lq_medieval/med_ebrick21.png", "start": 280983368, "end": 280995510}, {"filename": "/GameData/textures/lq_medieval/med_ebrick22.png", "start": 280995510, "end": 281007671}, {"filename": "/GameData/textures/lq_medieval/med_ebrick3.png", "start": 281007671, "end": 281020131}, {"filename": "/GameData/textures/lq_medieval/med_ebrick4.png", "start": 281020131, "end": 281035681}, {"filename": "/GameData/textures/lq_medieval/med_ebrick5.png", "start": 281035681, "end": 281049497}, {"filename": "/GameData/textures/lq_medieval/med_ebrick6.png", "start": 281049497, "end": 281064208}, {"filename": "/GameData/textures/lq_medieval/med_ebrick7.png", "start": 281064208, "end": 281077853}, {"filename": "/GameData/textures/lq_medieval/med_ebrick8.png", "start": 281077853, "end": 281090592}, {"filename": "/GameData/textures/lq_medieval/med_ebrick9.png", "start": 281090592, "end": 281105147}, {"filename": "/GameData/textures/lq_medieval/med_etrim1.png", "start": 281105147, "end": 281108476}, {"filename": "/GameData/textures/lq_medieval/med_glass1.png", "start": 281108476, "end": 281120855}, {"filename": "/GameData/textures/lq_medieval/med_glass2.png", "start": 281120855, "end": 281132512}, {"filename": "/GameData/textures/lq_medieval/med_glass3.png", "start": 281132512, "end": 281143527}, {"filename": "/GameData/textures/lq_medieval/med_glass4.png", "start": 281143527, "end": 281175346}, {"filename": "/GameData/textures/lq_medieval/med_glass5.png", "start": 281175346, "end": 281185128}, {"filename": "/GameData/textures/lq_medieval/med_met_dec1.png", "start": 281185128, "end": 281189268}, {"filename": "/GameData/textures/lq_medieval/med_met_key1a.png", "start": 281189268, "end": 281191028}, {"filename": "/GameData/textures/lq_medieval/med_met_key1b.png", "start": 281191028, "end": 281192362}, {"filename": "/GameData/textures/lq_medieval/med_met_key2a.png", "start": 281192362, "end": 281194122}, {"filename": "/GameData/textures/lq_medieval/med_met_key2b.png", "start": 281194122, "end": 281195828}, {"filename": "/GameData/textures/lq_medieval/med_met_trim1.png", "start": 281195828, "end": 281196855}, {"filename": "/GameData/textures/lq_medieval/med_met_trim2.png", "start": 281196855, "end": 281197802}, {"filename": "/GameData/textures/lq_medieval/med_met_trim3.png", "start": 281197802, "end": 281198676}, {"filename": "/GameData/textures/lq_medieval/med_metw1a.png", "start": 281198676, "end": 281210984}, {"filename": "/GameData/textures/lq_medieval/med_metw1b.png", "start": 281210984, "end": 281219691}, {"filename": "/GameData/textures/lq_medieval/med_metw2a.png", "start": 281219691, "end": 281229598}, {"filename": "/GameData/textures/lq_medieval/med_metw2b.png", "start": 281229598, "end": 281241976}, {"filename": "/GameData/textures/lq_medieval/med_rmet.png", "start": 281241976, "end": 281255077}, {"filename": "/GameData/textures/lq_medieval/med_rmet_slat.png", "start": 281255077, "end": 281269388}, {"filename": "/GameData/textures/lq_medieval/med_rmet_tile.png", "start": 281269388, "end": 281283320}, {"filename": "/GameData/textures/lq_medieval/med_rmet_trim32.png", "start": 281283320, "end": 281296529}, {"filename": "/GameData/textures/lq_medieval/med_roof1.png", "start": 281296529, "end": 281308444}, {"filename": "/GameData/textures/lq_medieval/med_roof2.png", "start": 281308444, "end": 281323517}, {"filename": "/GameData/textures/lq_medieval/med_roof3.png", "start": 281323517, "end": 281338446}, {"filename": "/GameData/textures/lq_medieval/med_roof4.png", "start": 281338446, "end": 281356260}, {"filename": "/GameData/textures/lq_medieval/med_roof5.png", "start": 281356260, "end": 281366766}, {"filename": "/GameData/textures/lq_medieval/med_tanwall1.png", "start": 281366766, "end": 281408444}, {"filename": "/GameData/textures/lq_medieval/med_tanwall2.png", "start": 281408444, "end": 281468221}, {"filename": "/GameData/textures/lq_medieval/med_tanwall3.png", "start": 281468221, "end": 281518656}, {"filename": "/GameData/textures/lq_medieval/med_tanwall4.png", "start": 281518656, "end": 281567797}, {"filename": "/GameData/textures/lq_medieval/med_tanwall4_f.png", "start": 281567797, "end": 281611750}, {"filename": "/GameData/textures/lq_medieval/med_tanwall6.png", "start": 281611750, "end": 281656295}, {"filename": "/GameData/textures/lq_medieval/med_tanwall7.png", "start": 281656295, "end": 281701888}, {"filename": "/GameData/textures/lq_medieval/med_tanwall8.png", "start": 281701888, "end": 281739486}, {"filename": "/GameData/textures/lq_medieval/med_tanwall9.png", "start": 281739486, "end": 281788566}, {"filename": "/GameData/textures/lq_medieval/med_tanwall9_f.png", "start": 281788566, "end": 281832545}, {"filename": "/GameData/textures/lq_medieval/med_telepad.png", "start": 281832545, "end": 281837172}, {"filename": "/GameData/textures/lq_medieval/med_tile1.png", "start": 281837172, "end": 281909783}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_lit1_fbr.png", "start": 281909783, "end": 281911386}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_lit3_fbr.png", "start": 281911386, "end": 281912691}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_tele.png", "start": 281912691, "end": 281915873}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim1.png", "start": 281915873, "end": 281919039}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim1b.png", "start": 281919039, "end": 281921094}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim2.png", "start": 281921094, "end": 281924634}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim3.png", "start": 281924634, "end": 281928111}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim4.png", "start": 281928111, "end": 281931224}, {"filename": "/GameData/textures/lq_medieval/med_trim1_1.png", "start": 281931224, "end": 281939928}, {"filename": "/GameData/textures/lq_medieval/med_trim2_1.png", "start": 281939928, "end": 281941042}, {"filename": "/GameData/textures/lq_medieval/med_trim3_1.png", "start": 281941042, "end": 281944637}, {"filename": "/GameData/textures/lq_medieval/med_trim3_2.png", "start": 281944637, "end": 281948196}, {"filename": "/GameData/textures/lq_medieval/med_trim3_3.png", "start": 281948196, "end": 281951893}, {"filename": "/GameData/textures/lq_medieval/med_trim3_4.png", "start": 281951893, "end": 281955264}, {"filename": "/GameData/textures/lq_medieval/med_trim4_1.png", "start": 281955264, "end": 281958688}, {"filename": "/GameData/textures/lq_medieval/med_trim4_2.png", "start": 281958688, "end": 281962212}, {"filename": "/GameData/textures/lq_medieval/med_trim4_3.png", "start": 281962212, "end": 281965572}, {"filename": "/GameData/textures/lq_medieval/med_trim4_4.png", "start": 281965572, "end": 281968795}, {"filename": "/GameData/textures/lq_medieval/plus_0_csl_brk14.png", "start": 281968795, "end": 281985108}, {"filename": "/GameData/textures/lq_medieval/plus_1_csl_brk14.png", "start": 281985108, "end": 282001401}, {"filename": "/GameData/textures/lq_medieval/plus_2_csl_brk14.png", "start": 282001401, "end": 282017700}, {"filename": "/GameData/textures/lq_medieval/plus_3_csl_brk14.png", "start": 282017700, "end": 282033942}, {"filename": "/GameData/textures/lq_medieval/plus_4_csl_brk14.png", "start": 282033942, "end": 282050283}, {"filename": "/GameData/textures/lq_medieval/sidewalk.png", "start": 282050283, "end": 282059039}, {"filename": "/GameData/textures/lq_medieval/sq_trim1_2.png", "start": 282059039, "end": 282067321}, {"filename": "/GameData/textures/lq_medieval/sq_trim1_2_s.png", "start": 282067321, "end": 282069896}, {"filename": "/GameData/textures/lq_medieval/tile.png", "start": 282069896, "end": 282073021}, {"filename": "/GameData/textures/lq_medieval/tile1.png", "start": 282073021, "end": 282145634}, {"filename": "/GameData/textures/lq_medieval/wall14_5.png", "start": 282145634, "end": 282149714}, {"filename": "/GameData/textures/lq_medieval/wbrick1_5.png", "start": 282149714, "end": 282153480}, {"filename": "/GameData/textures/lq_medieval/wswamp2_1.png", "start": 282153480, "end": 282156606}, {"filename": "/GameData/textures/lq_medieval/wswamp2_2.png", "start": 282156606, "end": 282160383}, {"filename": "/GameData/textures/lq_metal/gig1_bone.png", "start": 282160383, "end": 282170064}, {"filename": "/GameData/textures/lq_metal/gig1_bone_l.png", "start": 282170064, "end": 282200725}, {"filename": "/GameData/textures/lq_metal/gig1_skull.png", "start": 282200725, "end": 282203389}, {"filename": "/GameData/textures/lq_metal/gig1_skull_l.png", "start": 282203389, "end": 282212174}, {"filename": "/GameData/textures/lq_metal/gig1_spine.png", "start": 282212174, "end": 282260875}, {"filename": "/GameData/textures/lq_metal/gig2_bone.png", "start": 282260875, "end": 282269298}, {"filename": "/GameData/textures/lq_metal/gig2_bone_l.png", "start": 282269298, "end": 282295046}, {"filename": "/GameData/textures/lq_metal/gig2_bone_s.png", "start": 282295046, "end": 282297659}, {"filename": "/GameData/textures/lq_metal/gig2_mouth_s.png", "start": 282297659, "end": 282300074}, {"filename": "/GameData/textures/lq_metal/med_flat8.png", "start": 282300074, "end": 282302932}, {"filename": "/GameData/textures/lq_metal/med_flat9.png", "start": 282302932, "end": 282305920}, {"filename": "/GameData/textures/lq_metal/met_blc_block.png", "start": 282305920, "end": 282309259}, {"filename": "/GameData/textures/lq_metal/met_blc_diam.png", "start": 282309259, "end": 282312149}, {"filename": "/GameData/textures/lq_metal/met_blc_trim28.png", "start": 282312149, "end": 282317192}, {"filename": "/GameData/textures/lq_metal/met_blc_trim32.png", "start": 282317192, "end": 282320338}, {"filename": "/GameData/textures/lq_metal/met_blc_trim32r.png", "start": 282320338, "end": 282323593}, {"filename": "/GameData/textures/lq_metal/met_blc_trim32s.png", "start": 282323593, "end": 282326693}, {"filename": "/GameData/textures/lq_metal/met_blc_trim64.png", "start": 282326693, "end": 282329836}, {"filename": "/GameData/textures/lq_metal/met_blu_block.png", "start": 282329836, "end": 282334856}, {"filename": "/GameData/textures/lq_metal/met_blu_det1.png", "start": 282334856, "end": 282336012}, {"filename": "/GameData/textures/lq_metal/met_blu_diam.png", "start": 282336012, "end": 282338909}, {"filename": "/GameData/textures/lq_metal/met_blu_diam2.png", "start": 282338909, "end": 282341704}, {"filename": "/GameData/textures/lq_metal/met_blu_diamc.png", "start": 282341704, "end": 282345235}, {"filename": "/GameData/textures/lq_metal/met_blu_door1.png", "start": 282345235, "end": 282349918}, {"filename": "/GameData/textures/lq_metal/met_blu_door2.png", "start": 282349918, "end": 282354417}, {"filename": "/GameData/textures/lq_metal/met_blu_door3.png", "start": 282354417, "end": 282359066}, {"filename": "/GameData/textures/lq_metal/met_blu_door4.png", "start": 282359066, "end": 282363630}, {"filename": "/GameData/textures/lq_metal/met_blu_door5.png", "start": 282363630, "end": 282366573}, {"filename": "/GameData/textures/lq_metal/met_blu_door6.png", "start": 282366573, "end": 282369169}, {"filename": "/GameData/textures/lq_metal/met_blu_fac1.png", "start": 282369169, "end": 282371571}, {"filename": "/GameData/textures/lq_metal/met_blu_flat.png", "start": 282371571, "end": 282375005}, {"filename": "/GameData/textures/lq_metal/met_blu_flatst.png", "start": 282375005, "end": 282377234}, {"filename": "/GameData/textures/lq_metal/met_blu_gig1.png", "start": 282377234, "end": 282381353}, {"filename": "/GameData/textures/lq_metal/met_blu_gig2.png", "start": 282381353, "end": 282384966}, {"filename": "/GameData/textures/lq_metal/met_blu_gig2b.png", "start": 282384966, "end": 282391160}, {"filename": "/GameData/textures/lq_metal/met_blu_grate.png", "start": 282391160, "end": 282394215}, {"filename": "/GameData/textures/lq_metal/met_blu_grate2.png", "start": 282394215, "end": 282397046}, {"filename": "/GameData/textures/lq_metal/met_blu_grate3.png", "start": 282397046, "end": 282398706}, {"filename": "/GameData/textures/lq_metal/met_blu_lit1_fbr.png", "start": 282398706, "end": 282400777}, {"filename": "/GameData/textures/lq_metal/met_blu_lit2_fbr.png", "start": 282400777, "end": 282401966}, {"filename": "/GameData/textures/lq_metal/met_blu_lit3.png", "start": 282401966, "end": 282403152}, {"filename": "/GameData/textures/lq_metal/met_blu_lit4.png", "start": 282403152, "end": 282404413}, {"filename": "/GameData/textures/lq_metal/met_blu_lit5.png", "start": 282404413, "end": 282406506}, {"filename": "/GameData/textures/lq_metal/met_blu_pan1.png", "start": 282406506, "end": 282409988}, {"filename": "/GameData/textures/lq_metal/met_blu_pan2.png", "start": 282409988, "end": 282412274}, {"filename": "/GameData/textures/lq_metal/met_blu_pan3.png", "start": 282412274, "end": 282415701}, {"filename": "/GameData/textures/lq_metal/met_blu_rect.png", "start": 282415701, "end": 282419480}, {"filename": "/GameData/textures/lq_metal/met_blu_rivg.png", "start": 282419480, "end": 282423401}, {"filename": "/GameData/textures/lq_metal/met_blu_rivs.png", "start": 282423401, "end": 282426759}, {"filename": "/GameData/textures/lq_metal/met_blu_slat.png", "start": 282426759, "end": 282430545}, {"filename": "/GameData/textures/lq_metal/met_blu_sqr.png", "start": 282430545, "end": 282434316}, {"filename": "/GameData/textures/lq_metal/met_blu_sqrd.png", "start": 282434316, "end": 282438154}, {"filename": "/GameData/textures/lq_metal/met_blu_sqrs.png", "start": 282438154, "end": 282441908}, {"filename": "/GameData/textures/lq_metal/met_blu_stile.png", "start": 282441908, "end": 282445135}, {"filename": "/GameData/textures/lq_metal/met_blu_tile.png", "start": 282445135, "end": 282448584}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16.png", "start": 282448584, "end": 282452288}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16g.png", "start": 282452288, "end": 282456038}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16h.png", "start": 282456038, "end": 282459655}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16s.png", "start": 282459655, "end": 282462823}, {"filename": "/GameData/textures/lq_metal/met_blu_trim28.png", "start": 282462823, "end": 282468120}, {"filename": "/GameData/textures/lq_metal/met_blu_trim32.png", "start": 282468120, "end": 282471570}, {"filename": "/GameData/textures/lq_metal/met_blu_trim32r.png", "start": 282471570, "end": 282475226}, {"filename": "/GameData/textures/lq_metal/met_blu_trim32s.png", "start": 282475226, "end": 282478550}, {"filename": "/GameData/textures/lq_metal/met_blu_trim64.png", "start": 282478550, "end": 282482027}, {"filename": "/GameData/textures/lq_metal/met_blu_vtrim.png", "start": 282482027, "end": 282485597}, {"filename": "/GameData/textures/lq_metal/met_brn2_pat.png", "start": 282485597, "end": 282489446}, {"filename": "/GameData/textures/lq_metal/met_brn_block.png", "start": 282489446, "end": 282493157}, {"filename": "/GameData/textures/lq_metal/met_brn_blockl.png", "start": 282493157, "end": 282496686}, {"filename": "/GameData/textures/lq_metal/met_brn_det1.png", "start": 282496686, "end": 282497849}, {"filename": "/GameData/textures/lq_metal/met_brn_flat.png", "start": 282497849, "end": 282501370}, {"filename": "/GameData/textures/lq_metal/met_brn_grate.png", "start": 282501370, "end": 282504758}, {"filename": "/GameData/textures/lq_metal/met_brn_grate2.png", "start": 282504758, "end": 282508264}, {"filename": "/GameData/textures/lq_metal/met_brn_grate3.png", "start": 282508264, "end": 282510282}, {"filename": "/GameData/textures/lq_metal/met_brn_lit1_fbr.png", "start": 282510282, "end": 282512395}, {"filename": "/GameData/textures/lq_metal/met_brn_lit2_fbr.png", "start": 282512395, "end": 282513583}, {"filename": "/GameData/textures/lq_metal/met_brn_lit3.png", "start": 282513583, "end": 282514754}, {"filename": "/GameData/textures/lq_metal/met_brn_lit4.png", "start": 282514754, "end": 282515964}, {"filename": "/GameData/textures/lq_metal/met_brn_lit5.png", "start": 282515964, "end": 282518017}, {"filename": "/GameData/textures/lq_metal/met_brn_pan1.png", "start": 282518017, "end": 282521480}, {"filename": "/GameData/textures/lq_metal/met_brn_pan2.png", "start": 282521480, "end": 282523756}, {"filename": "/GameData/textures/lq_metal/met_brn_pan3.png", "start": 282523756, "end": 282527233}, {"filename": "/GameData/textures/lq_metal/met_brn_pan4.png", "start": 282527233, "end": 282531096}, {"filename": "/GameData/textures/lq_metal/met_brn_rect.png", "start": 282531096, "end": 282534774}, {"filename": "/GameData/textures/lq_metal/met_brn_rivg.png", "start": 282534774, "end": 282538802}, {"filename": "/GameData/textures/lq_metal/met_brn_rivs.png", "start": 282538802, "end": 282542225}, {"filename": "/GameData/textures/lq_metal/met_brn_signs.png", "start": 282542225, "end": 282547059}, {"filename": "/GameData/textures/lq_metal/met_brn_slat.png", "start": 282547059, "end": 282550702}, {"filename": "/GameData/textures/lq_metal/met_brn_sqr.png", "start": 282550702, "end": 282554393}, {"filename": "/GameData/textures/lq_metal/met_brn_sqrd.png", "start": 282554393, "end": 282558201}, {"filename": "/GameData/textures/lq_metal/met_brn_sqrs.png", "start": 282558201, "end": 282561905}, {"filename": "/GameData/textures/lq_metal/met_brn_stile.png", "start": 282561905, "end": 282565990}, {"filename": "/GameData/textures/lq_metal/met_brn_tile.png", "start": 282565990, "end": 282569371}, {"filename": "/GameData/textures/lq_metal/met_brn_tile2.png", "start": 282569371, "end": 282572704}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16.png", "start": 282572704, "end": 282576810}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16g.png", "start": 282576810, "end": 282581014}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16h.png", "start": 282581014, "end": 282585211}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16s.png", "start": 282585211, "end": 282588834}, {"filename": "/GameData/textures/lq_metal/met_brn_trim32.png", "start": 282588834, "end": 282592321}, {"filename": "/GameData/textures/lq_metal/met_brn_trim32s.png", "start": 282592321, "end": 282595707}, {"filename": "/GameData/textures/lq_metal/met_brn_trim64.png", "start": 282595707, "end": 282599347}, {"filename": "/GameData/textures/lq_metal/met_brn_vtrim.png", "start": 282599347, "end": 282602800}, {"filename": "/GameData/textures/lq_metal/met_cop_flat.png", "start": 282602800, "end": 282605877}, {"filename": "/GameData/textures/lq_metal/met_cop_riv.png", "start": 282605877, "end": 282609478}, {"filename": "/GameData/textures/lq_metal/met_dbrn_flat.png", "start": 282609478, "end": 282612858}, {"filename": "/GameData/textures/lq_metal/met_dbrn_rect.png", "start": 282612858, "end": 282616743}, {"filename": "/GameData/textures/lq_metal/met_dbrn_slat.png", "start": 282616743, "end": 282620558}, {"filename": "/GameData/textures/lq_metal/met_grate.png", "start": 282620558, "end": 282623908}, {"filename": "/GameData/textures/lq_metal/met_grn_block.png", "start": 282623908, "end": 282627833}, {"filename": "/GameData/textures/lq_metal/met_grn_blockl.png", "start": 282627833, "end": 282631562}, {"filename": "/GameData/textures/lq_metal/met_grn_det1.png", "start": 282631562, "end": 282632813}, {"filename": "/GameData/textures/lq_metal/met_grn_fac1.png", "start": 282632813, "end": 282635723}, {"filename": "/GameData/textures/lq_metal/met_grn_flat.png", "start": 282635723, "end": 282639608}, {"filename": "/GameData/textures/lq_metal/met_grn_grate.png", "start": 282639608, "end": 282642670}, {"filename": "/GameData/textures/lq_metal/met_grn_grate2.png", "start": 282642670, "end": 282645653}, {"filename": "/GameData/textures/lq_metal/met_grn_grate3.png", "start": 282645653, "end": 282647397}, {"filename": "/GameData/textures/lq_metal/met_grn_lit1_fbr.png", "start": 282647397, "end": 282649531}, {"filename": "/GameData/textures/lq_metal/met_grn_lit2_fbr.png", "start": 282649531, "end": 282650732}, {"filename": "/GameData/textures/lq_metal/met_grn_lit3.png", "start": 282650732, "end": 282651944}, {"filename": "/GameData/textures/lq_metal/met_grn_lit4.png", "start": 282651944, "end": 282653230}, {"filename": "/GameData/textures/lq_metal/met_grn_lit5.png", "start": 282653230, "end": 282655422}, {"filename": "/GameData/textures/lq_metal/met_grn_pan1.png", "start": 282655422, "end": 282659138}, {"filename": "/GameData/textures/lq_metal/met_grn_pan2.png", "start": 282659138, "end": 282661503}, {"filename": "/GameData/textures/lq_metal/met_grn_pan3.png", "start": 282661503, "end": 282665103}, {"filename": "/GameData/textures/lq_metal/met_grn_rect.png", "start": 282665103, "end": 282669140}, {"filename": "/GameData/textures/lq_metal/met_grn_rivg.png", "start": 282669140, "end": 282673552}, {"filename": "/GameData/textures/lq_metal/met_grn_rivs.png", "start": 282673552, "end": 282677319}, {"filename": "/GameData/textures/lq_metal/met_grn_slat.png", "start": 282677319, "end": 282681360}, {"filename": "/GameData/textures/lq_metal/met_grn_sqr.png", "start": 282681360, "end": 282685358}, {"filename": "/GameData/textures/lq_metal/met_grn_sqrd.png", "start": 282685358, "end": 282689468}, {"filename": "/GameData/textures/lq_metal/met_grn_sqrs.png", "start": 282689468, "end": 282693498}, {"filename": "/GameData/textures/lq_metal/met_grn_stile.png", "start": 282693498, "end": 282696782}, {"filename": "/GameData/textures/lq_metal/met_grn_tile.png", "start": 282696782, "end": 282700043}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16.png", "start": 282700043, "end": 282703655}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16g.png", "start": 282703655, "end": 282707809}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16h.png", "start": 282707809, "end": 282711358}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16s.png", "start": 282711358, "end": 282714869}, {"filename": "/GameData/textures/lq_metal/met_grn_trim28.png", "start": 282714869, "end": 282719804}, {"filename": "/GameData/textures/lq_metal/met_grn_trim28r.png", "start": 282719804, "end": 282724830}, {"filename": "/GameData/textures/lq_metal/met_grn_trim32.png", "start": 282724830, "end": 282728655}, {"filename": "/GameData/textures/lq_metal/met_grn_trim32r.png", "start": 282728655, "end": 282732525}, {"filename": "/GameData/textures/lq_metal/met_grn_trim32s.png", "start": 282732525, "end": 282736171}, {"filename": "/GameData/textures/lq_metal/met_grn_trim64.png", "start": 282736171, "end": 282739828}, {"filename": "/GameData/textures/lq_metal/met_grn_vtrim.png", "start": 282739828, "end": 282743078}, {"filename": "/GameData/textures/lq_metal/met_gry_beam.png", "start": 282743078, "end": 282746092}, {"filename": "/GameData/textures/lq_metal/met_gry_block.png", "start": 282746092, "end": 282749115}, {"filename": "/GameData/textures/lq_metal/met_gry_flat.png", "start": 282749115, "end": 282751849}, {"filename": "/GameData/textures/lq_metal/met_gry_lit1_fbr.png", "start": 282751849, "end": 282753660}, {"filename": "/GameData/textures/lq_metal/met_gry_lit2_fbr.png", "start": 282753660, "end": 282754783}, {"filename": "/GameData/textures/lq_metal/met_gry_lit2b.png", "start": 282754783, "end": 282758233}, {"filename": "/GameData/textures/lq_metal/met_gry_pan1.png", "start": 282758233, "end": 282761329}, {"filename": "/GameData/textures/lq_metal/met_gry_pan2.png", "start": 282761329, "end": 282763482}, {"filename": "/GameData/textures/lq_metal/met_gry_pan3.png", "start": 282763482, "end": 282766585}, {"filename": "/GameData/textures/lq_metal/met_gry_rect.png", "start": 282766585, "end": 282769782}, {"filename": "/GameData/textures/lq_metal/met_gry_signs.png", "start": 282769782, "end": 282773612}, {"filename": "/GameData/textures/lq_metal/met_gry_slat.png", "start": 282773612, "end": 282777028}, {"filename": "/GameData/textures/lq_metal/met_gry_sqr.png", "start": 282777028, "end": 282779862}, {"filename": "/GameData/textures/lq_metal/met_gry_sqrd.png", "start": 282779862, "end": 282783110}, {"filename": "/GameData/textures/lq_metal/met_gry_sqrs.png", "start": 282783110, "end": 282786071}, {"filename": "/GameData/textures/lq_metal/met_gry_trim64.png", "start": 282786071, "end": 282789772}, {"filename": "/GameData/textures/lq_metal/met_lbrn_flat.png", "start": 282789772, "end": 282793198}, {"filename": "/GameData/textures/lq_metal/met_lbrn_rect.png", "start": 282793198, "end": 282797086}, {"filename": "/GameData/textures/lq_metal/met_lbrn_slat.png", "start": 282797086, "end": 282801040}, {"filename": "/GameData/textures/lq_metal/met_lift.png", "start": 282801040, "end": 282804085}, {"filename": "/GameData/textures/lq_metal/met_met7_1.png", "start": 282804085, "end": 282806558}, {"filename": "/GameData/textures/lq_metal/met_mix_beam.png", "start": 282806558, "end": 282810210}, {"filename": "/GameData/textures/lq_metal/met_mix_diam.png", "start": 282810210, "end": 282813399}, {"filename": "/GameData/textures/lq_metal/met_mix_diam2.png", "start": 282813399, "end": 282816598}, {"filename": "/GameData/textures/lq_metal/met_mix_diamc.png", "start": 282816598, "end": 282820698}, {"filename": "/GameData/textures/lq_metal/met_mt1_flat.png", "start": 282820698, "end": 282834988}, {"filename": "/GameData/textures/lq_metal/met_mt1_rect.png", "start": 282834988, "end": 282850698}, {"filename": "/GameData/textures/lq_metal/met_mt1_slat.png", "start": 282850698, "end": 282866454}, {"filename": "/GameData/textures/lq_metal/met_mt1_sqr.png", "start": 282866454, "end": 282872994}, {"filename": "/GameData/textures/lq_metal/met_mt2_flat.png", "start": 282872994, "end": 282889208}, {"filename": "/GameData/textures/lq_metal/met_mt2_rect.png", "start": 282889208, "end": 282905934}, {"filename": "/GameData/textures/lq_metal/met_mt2_slat.png", "start": 282905934, "end": 282922572}, {"filename": "/GameData/textures/lq_metal/met_mt2_sqr.png", "start": 282922572, "end": 282930970}, {"filename": "/GameData/textures/lq_metal/met_mt3_flat.png", "start": 282930970, "end": 282947570}, {"filename": "/GameData/textures/lq_metal/met_mt3_rect.png", "start": 282947570, "end": 282964478}, {"filename": "/GameData/textures/lq_metal/met_mt3_slat.png", "start": 282964478, "end": 282981389}, {"filename": "/GameData/textures/lq_metal/met_mt3_sqr.png", "start": 282981389, "end": 282989924}, {"filename": "/GameData/textures/lq_metal/met_ora_trim64.png", "start": 282989924, "end": 282992703}, {"filename": "/GameData/textures/lq_metal/met_rail_flat.png", "start": 282992703, "end": 282994850}, {"filename": "/GameData/textures/lq_metal/met_rune1_fbr.png", "start": 282994850, "end": 282997654}, {"filename": "/GameData/textures/lq_metal/met_rune_trim32.png", "start": 282997654, "end": 283001995}, {"filename": "/GameData/textures/lq_metal/met_set1.png", "start": 283001995, "end": 283071840}, {"filename": "/GameData/textures/lq_metal/met_shm_flat.png", "start": 283071840, "end": 283075501}, {"filename": "/GameData/textures/lq_metal/met_shm_rect.png", "start": 283075501, "end": 283079373}, {"filename": "/GameData/textures/lq_metal/met_shm_slat.png", "start": 283079373, "end": 283083424}, {"filename": "/GameData/textures/lq_metal/met_shm_sqr.png", "start": 283083424, "end": 283086886}, {"filename": "/GameData/textures/lq_metal/met_teal_block.png", "start": 283086886, "end": 283090264}, {"filename": "/GameData/textures/lq_metal/met_teal_flat.png", "start": 283090264, "end": 283093610}, {"filename": "/GameData/textures/lq_metal/met_teal_trim32.png", "start": 283093610, "end": 283096886}, {"filename": "/GameData/textures/lq_metal/met_teal_trim32r.png", "start": 283096886, "end": 283099861}, {"filename": "/GameData/textures/lq_metal/met_teal_trim64.png", "start": 283099861, "end": 283103345}, {"filename": "/GameData/textures/lq_metal/met_wall3_1.png", "start": 283103345, "end": 283114611}, {"filename": "/GameData/textures/lq_metal/met_wall3_1_s.png", "start": 283114611, "end": 283117895}, {"filename": "/GameData/textures/lq_metal/metal4_4.png", "start": 283117895, "end": 283122236}, {"filename": "/GameData/textures/lq_metal/plus_0_sqbut1.png", "start": 283122236, "end": 283123318}, {"filename": "/GameData/textures/lq_metal/plus_0_sqbut2_fbr.png", "start": 283123318, "end": 283126554}, {"filename": "/GameData/textures/lq_metal/plus_0_sqshoot1_fbr.png", "start": 283126554, "end": 283127661}, {"filename": "/GameData/textures/lq_metal/plus_0gig2a_fbr.png", "start": 283127661, "end": 283128134}, {"filename": "/GameData/textures/lq_metal/plus_0gig_shot_fbr.png", "start": 283128134, "end": 283128610}, {"filename": "/GameData/textures/lq_metal/plus_0gig_sshot_fbr.png", "start": 283128610, "end": 283129054}, {"filename": "/GameData/textures/lq_metal/plus_0gig_ye_fbr.png", "start": 283129054, "end": 283129494}, {"filename": "/GameData/textures/lq_metal/plus_0met_blu_keyg_fbr.png", "start": 283129494, "end": 283130394}, {"filename": "/GameData/textures/lq_metal/plus_0met_blu_keys_fbr.png", "start": 283130394, "end": 283131277}, {"filename": "/GameData/textures/lq_metal/plus_1_sqbut1.png", "start": 283131277, "end": 283132443}, {"filename": "/GameData/textures/lq_metal/plus_1_sqbut2_fbr.png", "start": 283132443, "end": 283135631}, {"filename": "/GameData/textures/lq_metal/plus_1_sqshoot1.png", "start": 283135631, "end": 283136745}, {"filename": "/GameData/textures/lq_metal/plus_1met_blu_keyg_fbr.png", "start": 283136745, "end": 283137646}, {"filename": "/GameData/textures/lq_metal/plus_1met_blu_keys_fbr.png", "start": 283137646, "end": 283138523}, {"filename": "/GameData/textures/lq_metal/plus_2met_blu_keyg_fbr.png", "start": 283138523, "end": 283139432}, {"filename": "/GameData/textures/lq_metal/plus_2met_blu_keys_fbr.png", "start": 283139432, "end": 283140301}, {"filename": "/GameData/textures/lq_metal/plus_3met_blu_keyg_fbr.png", "start": 283140301, "end": 283141209}, {"filename": "/GameData/textures/lq_metal/plus_3met_blu_keys_fbr.png", "start": 283141209, "end": 283142065}, {"filename": "/GameData/textures/lq_metal/plus_4met_blu_keyg_fbr.png", "start": 283142065, "end": 283142973}, {"filename": "/GameData/textures/lq_metal/plus_4met_blu_keys_fbr.png", "start": 283142973, "end": 283143829}, {"filename": "/GameData/textures/lq_metal/plus_5met_blu_keyg_fbr.png", "start": 283143829, "end": 283144738}, {"filename": "/GameData/textures/lq_metal/plus_5met_blu_keys_fbr.png", "start": 283144738, "end": 283145607}, {"filename": "/GameData/textures/lq_metal/plus_6met_blu_keyg_fbr.png", "start": 283145607, "end": 283146508}, {"filename": "/GameData/textures/lq_metal/plus_6met_blu_keys_fbr.png", "start": 283146508, "end": 283147385}, {"filename": "/GameData/textures/lq_metal/plus_a_sqbut1.png", "start": 283147385, "end": 283148467}, {"filename": "/GameData/textures/lq_metal/plus_a_sqbut2_fbr.png", "start": 283148467, "end": 283151703}, {"filename": "/GameData/textures/lq_metal/plus_a_sqshoot1_fbr.png", "start": 283151703, "end": 283152810}, {"filename": "/GameData/textures/lq_metal/plus_agig2a.png", "start": 283152810, "end": 283153180}, {"filename": "/GameData/textures/lq_metal/plus_agig_shot_fbr.png", "start": 283153180, "end": 283153555}, {"filename": "/GameData/textures/lq_metal/plus_agig_sshot_fbr.png", "start": 283153555, "end": 283153918}, {"filename": "/GameData/textures/lq_metal/plus_agig_ye.png", "start": 283153918, "end": 283154330}, {"filename": "/GameData/textures/lq_metal/plus_amet_blu_keyg.png", "start": 283154330, "end": 283155200}, {"filename": "/GameData/textures/lq_metal/plus_amet_blu_keys.png", "start": 283155200, "end": 283156070}, {"filename": "/GameData/textures/lq_metal/ret_metal1_tile.png", "start": 283156070, "end": 283170002}, {"filename": "/GameData/textures/lq_metal/sq_lit1_fbr.png", "start": 283170002, "end": 283170375}, {"filename": "/GameData/textures/lq_metal/sq_lit2_fbr.png", "start": 283170375, "end": 283170636}, {"filename": "/GameData/textures/lq_palette/flat_01_a.png", "start": 283170636, "end": 283171184}, {"filename": "/GameData/textures/lq_palette/flat_01_b.png", "start": 283171184, "end": 283171710}, {"filename": "/GameData/textures/lq_palette/flat_01_c.png", "start": 283171710, "end": 283172236}, {"filename": "/GameData/textures/lq_palette/flat_01_d.png", "start": 283172236, "end": 283172762}, {"filename": "/GameData/textures/lq_palette/flat_01_e.png", "start": 283172762, "end": 283173288}, {"filename": "/GameData/textures/lq_palette/flat_01_f.png", "start": 283173288, "end": 283173814}, {"filename": "/GameData/textures/lq_palette/flat_01_g.png", "start": 283173814, "end": 283174340}, {"filename": "/GameData/textures/lq_palette/flat_01_h.png", "start": 283174340, "end": 283174866}, {"filename": "/GameData/textures/lq_palette/flat_01_i.png", "start": 283174866, "end": 283175393}, {"filename": "/GameData/textures/lq_palette/flat_01_j.png", "start": 283175393, "end": 283175920}, {"filename": "/GameData/textures/lq_palette/flat_01_k.png", "start": 283175920, "end": 283176447}, {"filename": "/GameData/textures/lq_palette/flat_01_l.png", "start": 283176447, "end": 283176974}, {"filename": "/GameData/textures/lq_palette/flat_01_m.png", "start": 283176974, "end": 283177501}, {"filename": "/GameData/textures/lq_palette/flat_01_n.png", "start": 283177501, "end": 283178026}, {"filename": "/GameData/textures/lq_palette/flat_01_o.png", "start": 283178026, "end": 283178551}, {"filename": "/GameData/textures/lq_palette/flat_01_p.png", "start": 283178551, "end": 283179076}, {"filename": "/GameData/textures/lq_palette/flat_02_a.png", "start": 283179076, "end": 283179602}, {"filename": "/GameData/textures/lq_palette/flat_02_b.png", "start": 283179602, "end": 283180128}, {"filename": "/GameData/textures/lq_palette/flat_02_c.png", "start": 283180128, "end": 283180654}, {"filename": "/GameData/textures/lq_palette/flat_02_d.png", "start": 283180654, "end": 283181180}, {"filename": "/GameData/textures/lq_palette/flat_02_e.png", "start": 283181180, "end": 283181706}, {"filename": "/GameData/textures/lq_palette/flat_02_f.png", "start": 283181706, "end": 283182232}, {"filename": "/GameData/textures/lq_palette/flat_02_g.png", "start": 283182232, "end": 283182758}, {"filename": "/GameData/textures/lq_palette/flat_02_h.png", "start": 283182758, "end": 283183284}, {"filename": "/GameData/textures/lq_palette/flat_02_i.png", "start": 283183284, "end": 283183810}, {"filename": "/GameData/textures/lq_palette/flat_02_j.png", "start": 283183810, "end": 283184336}, {"filename": "/GameData/textures/lq_palette/flat_02_k.png", "start": 283184336, "end": 283184862}, {"filename": "/GameData/textures/lq_palette/flat_02_l.png", "start": 283184862, "end": 283185388}, {"filename": "/GameData/textures/lq_palette/flat_02_m.png", "start": 283185388, "end": 283185914}, {"filename": "/GameData/textures/lq_palette/flat_02_n.png", "start": 283185914, "end": 283186440}, {"filename": "/GameData/textures/lq_palette/flat_02_o.png", "start": 283186440, "end": 283186966}, {"filename": "/GameData/textures/lq_palette/flat_02_p.png", "start": 283186966, "end": 283187492}, {"filename": "/GameData/textures/lq_palette/flat_03_a.png", "start": 283187492, "end": 283188018}, {"filename": "/GameData/textures/lq_palette/flat_03_b.png", "start": 283188018, "end": 283188544}, {"filename": "/GameData/textures/lq_palette/flat_03_c.png", "start": 283188544, "end": 283189070}, {"filename": "/GameData/textures/lq_palette/flat_03_d.png", "start": 283189070, "end": 283189596}, {"filename": "/GameData/textures/lq_palette/flat_03_e.png", "start": 283189596, "end": 283190122}, {"filename": "/GameData/textures/lq_palette/flat_03_f.png", "start": 283190122, "end": 283190648}, {"filename": "/GameData/textures/lq_palette/flat_03_g.png", "start": 283190648, "end": 283191174}, {"filename": "/GameData/textures/lq_palette/flat_03_h.png", "start": 283191174, "end": 283191700}, {"filename": "/GameData/textures/lq_palette/flat_03_i.png", "start": 283191700, "end": 283192226}, {"filename": "/GameData/textures/lq_palette/flat_03_j.png", "start": 283192226, "end": 283192752}, {"filename": "/GameData/textures/lq_palette/flat_03_k.png", "start": 283192752, "end": 283193278}, {"filename": "/GameData/textures/lq_palette/flat_03_l.png", "start": 283193278, "end": 283193804}, {"filename": "/GameData/textures/lq_palette/flat_03_m.png", "start": 283193804, "end": 283194331}, {"filename": "/GameData/textures/lq_palette/flat_03_n.png", "start": 283194331, "end": 283194858}, {"filename": "/GameData/textures/lq_palette/flat_03_o.png", "start": 283194858, "end": 283195385}, {"filename": "/GameData/textures/lq_palette/flat_03_p.png", "start": 283195385, "end": 283195912}, {"filename": "/GameData/textures/lq_palette/flat_04_a.png", "start": 283195912, "end": 283196460}, {"filename": "/GameData/textures/lq_palette/flat_04_b.png", "start": 283196460, "end": 283196986}, {"filename": "/GameData/textures/lq_palette/flat_04_c.png", "start": 283196986, "end": 283197512}, {"filename": "/GameData/textures/lq_palette/flat_04_d.png", "start": 283197512, "end": 283198038}, {"filename": "/GameData/textures/lq_palette/flat_04_e.png", "start": 283198038, "end": 283198564}, {"filename": "/GameData/textures/lq_palette/flat_04_f.png", "start": 283198564, "end": 283199090}, {"filename": "/GameData/textures/lq_palette/flat_04_g.png", "start": 283199090, "end": 283199616}, {"filename": "/GameData/textures/lq_palette/flat_04_h.png", "start": 283199616, "end": 283200142}, {"filename": "/GameData/textures/lq_palette/flat_04_i.png", "start": 283200142, "end": 283200668}, {"filename": "/GameData/textures/lq_palette/flat_04_j.png", "start": 283200668, "end": 283201194}, {"filename": "/GameData/textures/lq_palette/flat_04_k.png", "start": 283201194, "end": 283201720}, {"filename": "/GameData/textures/lq_palette/flat_04_l.png", "start": 283201720, "end": 283202246}, {"filename": "/GameData/textures/lq_palette/flat_04_m.png", "start": 283202246, "end": 283202772}, {"filename": "/GameData/textures/lq_palette/flat_04_n.png", "start": 283202772, "end": 283203298}, {"filename": "/GameData/textures/lq_palette/flat_04_o.png", "start": 283203298, "end": 283203824}, {"filename": "/GameData/textures/lq_palette/flat_04_p.png", "start": 283203824, "end": 283204350}, {"filename": "/GameData/textures/lq_palette/flat_05_a.png", "start": 283204350, "end": 283204876}, {"filename": "/GameData/textures/lq_palette/flat_05_b.png", "start": 283204876, "end": 283205402}, {"filename": "/GameData/textures/lq_palette/flat_05_c.png", "start": 283205402, "end": 283205928}, {"filename": "/GameData/textures/lq_palette/flat_05_d.png", "start": 283205928, "end": 283206454}, {"filename": "/GameData/textures/lq_palette/flat_05_e.png", "start": 283206454, "end": 283206980}, {"filename": "/GameData/textures/lq_palette/flat_05_f.png", "start": 283206980, "end": 283207506}, {"filename": "/GameData/textures/lq_palette/flat_05_g.png", "start": 283207506, "end": 283208032}, {"filename": "/GameData/textures/lq_palette/flat_05_h.png", "start": 283208032, "end": 283208558}, {"filename": "/GameData/textures/lq_palette/flat_05_i.png", "start": 283208558, "end": 283209084}, {"filename": "/GameData/textures/lq_palette/flat_05_j.png", "start": 283209084, "end": 283209610}, {"filename": "/GameData/textures/lq_palette/flat_05_k.png", "start": 283209610, "end": 283210136}, {"filename": "/GameData/textures/lq_palette/flat_05_l.png", "start": 283210136, "end": 283210662}, {"filename": "/GameData/textures/lq_palette/flat_05_m.png", "start": 283210662, "end": 283211188}, {"filename": "/GameData/textures/lq_palette/flat_05_n.png", "start": 283211188, "end": 283211714}, {"filename": "/GameData/textures/lq_palette/flat_05_o.png", "start": 283211714, "end": 283212240}, {"filename": "/GameData/textures/lq_palette/flat_05_p.png", "start": 283212240, "end": 283212766}, {"filename": "/GameData/textures/lq_palette/flat_06_a.png", "start": 283212766, "end": 283213292}, {"filename": "/GameData/textures/lq_palette/flat_06_b.png", "start": 283213292, "end": 283213818}, {"filename": "/GameData/textures/lq_palette/flat_06_c.png", "start": 283213818, "end": 283214344}, {"filename": "/GameData/textures/lq_palette/flat_06_d.png", "start": 283214344, "end": 283214870}, {"filename": "/GameData/textures/lq_palette/flat_06_e.png", "start": 283214870, "end": 283215396}, {"filename": "/GameData/textures/lq_palette/flat_06_f.png", "start": 283215396, "end": 283215922}, {"filename": "/GameData/textures/lq_palette/flat_06_g.png", "start": 283215922, "end": 283216448}, {"filename": "/GameData/textures/lq_palette/flat_06_h.png", "start": 283216448, "end": 283216974}, {"filename": "/GameData/textures/lq_palette/flat_06_i.png", "start": 283216974, "end": 283217500}, {"filename": "/GameData/textures/lq_palette/flat_06_j.png", "start": 283217500, "end": 283218026}, {"filename": "/GameData/textures/lq_palette/flat_06_k.png", "start": 283218026, "end": 283218552}, {"filename": "/GameData/textures/lq_palette/flat_06_l.png", "start": 283218552, "end": 283219078}, {"filename": "/GameData/textures/lq_palette/flat_06_m.png", "start": 283219078, "end": 283219604}, {"filename": "/GameData/textures/lq_palette/flat_06_n.png", "start": 283219604, "end": 283220130}, {"filename": "/GameData/textures/lq_palette/flat_06_o.png", "start": 283220130, "end": 283220656}, {"filename": "/GameData/textures/lq_palette/flat_06_p.png", "start": 283220656, "end": 283221182}, {"filename": "/GameData/textures/lq_palette/flat_07_a.png", "start": 283221182, "end": 283221708}, {"filename": "/GameData/textures/lq_palette/flat_07_b.png", "start": 283221708, "end": 283222234}, {"filename": "/GameData/textures/lq_palette/flat_07_c.png", "start": 283222234, "end": 283222760}, {"filename": "/GameData/textures/lq_palette/flat_07_d.png", "start": 283222760, "end": 283223286}, {"filename": "/GameData/textures/lq_palette/flat_07_e.png", "start": 283223286, "end": 283223812}, {"filename": "/GameData/textures/lq_palette/flat_07_f.png", "start": 283223812, "end": 283224338}, {"filename": "/GameData/textures/lq_palette/flat_07_g.png", "start": 283224338, "end": 283224864}, {"filename": "/GameData/textures/lq_palette/flat_07_h.png", "start": 283224864, "end": 283225390}, {"filename": "/GameData/textures/lq_palette/flat_07_i.png", "start": 283225390, "end": 283225916}, {"filename": "/GameData/textures/lq_palette/flat_07_j.png", "start": 283225916, "end": 283226442}, {"filename": "/GameData/textures/lq_palette/flat_07_k.png", "start": 283226442, "end": 283226968}, {"filename": "/GameData/textures/lq_palette/flat_07_l.png", "start": 283226968, "end": 283227494}, {"filename": "/GameData/textures/lq_palette/flat_07_m.png", "start": 283227494, "end": 283228020}, {"filename": "/GameData/textures/lq_palette/flat_07_n.png", "start": 283228020, "end": 283228546}, {"filename": "/GameData/textures/lq_palette/flat_07_o.png", "start": 283228546, "end": 283229072}, {"filename": "/GameData/textures/lq_palette/flat_07_p.png", "start": 283229072, "end": 283229598}, {"filename": "/GameData/textures/lq_palette/flat_08_a.png", "start": 283229598, "end": 283230124}, {"filename": "/GameData/textures/lq_palette/flat_08_b.png", "start": 283230124, "end": 283230650}, {"filename": "/GameData/textures/lq_palette/flat_08_c.png", "start": 283230650, "end": 283231176}, {"filename": "/GameData/textures/lq_palette/flat_08_d.png", "start": 283231176, "end": 283231702}, {"filename": "/GameData/textures/lq_palette/flat_08_e.png", "start": 283231702, "end": 283232228}, {"filename": "/GameData/textures/lq_palette/flat_08_f.png", "start": 283232228, "end": 283232754}, {"filename": "/GameData/textures/lq_palette/flat_08_g.png", "start": 283232754, "end": 283233280}, {"filename": "/GameData/textures/lq_palette/flat_08_h.png", "start": 283233280, "end": 283233806}, {"filename": "/GameData/textures/lq_palette/flat_08_i.png", "start": 283233806, "end": 283234332}, {"filename": "/GameData/textures/lq_palette/flat_08_j.png", "start": 283234332, "end": 283234858}, {"filename": "/GameData/textures/lq_palette/flat_08_k.png", "start": 283234858, "end": 283235384}, {"filename": "/GameData/textures/lq_palette/flat_08_l.png", "start": 283235384, "end": 283235910}, {"filename": "/GameData/textures/lq_palette/flat_08_m.png", "start": 283235910, "end": 283236436}, {"filename": "/GameData/textures/lq_palette/flat_08_n.png", "start": 283236436, "end": 283236963}, {"filename": "/GameData/textures/lq_palette/flat_08_o.png", "start": 283236963, "end": 283237490}, {"filename": "/GameData/textures/lq_palette/flat_08_p.png", "start": 283237490, "end": 283238017}, {"filename": "/GameData/textures/lq_palette/flat_09_a.png", "start": 283238017, "end": 283238544}, {"filename": "/GameData/textures/lq_palette/flat_09_b.png", "start": 283238544, "end": 283239071}, {"filename": "/GameData/textures/lq_palette/flat_09_c.png", "start": 283239071, "end": 283239598}, {"filename": "/GameData/textures/lq_palette/flat_09_d.png", "start": 283239598, "end": 283240124}, {"filename": "/GameData/textures/lq_palette/flat_09_e.png", "start": 283240124, "end": 283240650}, {"filename": "/GameData/textures/lq_palette/flat_09_f.png", "start": 283240650, "end": 283241176}, {"filename": "/GameData/textures/lq_palette/flat_09_g.png", "start": 283241176, "end": 283241702}, {"filename": "/GameData/textures/lq_palette/flat_09_h.png", "start": 283241702, "end": 283242228}, {"filename": "/GameData/textures/lq_palette/flat_09_i.png", "start": 283242228, "end": 283242754}, {"filename": "/GameData/textures/lq_palette/flat_09_j.png", "start": 283242754, "end": 283243280}, {"filename": "/GameData/textures/lq_palette/flat_09_k.png", "start": 283243280, "end": 283243806}, {"filename": "/GameData/textures/lq_palette/flat_09_l.png", "start": 283243806, "end": 283244332}, {"filename": "/GameData/textures/lq_palette/flat_09_m.png", "start": 283244332, "end": 283244858}, {"filename": "/GameData/textures/lq_palette/flat_09_n.png", "start": 283244858, "end": 283245384}, {"filename": "/GameData/textures/lq_palette/flat_09_o.png", "start": 283245384, "end": 283245910}, {"filename": "/GameData/textures/lq_palette/flat_09_p.png", "start": 283245910, "end": 283246436}, {"filename": "/GameData/textures/lq_palette/flat_10_a.png", "start": 283246436, "end": 283246963}, {"filename": "/GameData/textures/lq_palette/flat_10_b.png", "start": 283246963, "end": 283247489}, {"filename": "/GameData/textures/lq_palette/flat_10_c.png", "start": 283247489, "end": 283248015}, {"filename": "/GameData/textures/lq_palette/flat_10_d.png", "start": 283248015, "end": 283248541}, {"filename": "/GameData/textures/lq_palette/flat_10_e.png", "start": 283248541, "end": 283249067}, {"filename": "/GameData/textures/lq_palette/flat_10_f.png", "start": 283249067, "end": 283249593}, {"filename": "/GameData/textures/lq_palette/flat_10_g.png", "start": 283249593, "end": 283250119}, {"filename": "/GameData/textures/lq_palette/flat_10_h.png", "start": 283250119, "end": 283250645}, {"filename": "/GameData/textures/lq_palette/flat_10_i.png", "start": 283250645, "end": 283251171}, {"filename": "/GameData/textures/lq_palette/flat_10_j.png", "start": 283251171, "end": 283251697}, {"filename": "/GameData/textures/lq_palette/flat_10_k.png", "start": 283251697, "end": 283252223}, {"filename": "/GameData/textures/lq_palette/flat_10_l.png", "start": 283252223, "end": 283252749}, {"filename": "/GameData/textures/lq_palette/flat_10_m.png", "start": 283252749, "end": 283253275}, {"filename": "/GameData/textures/lq_palette/flat_10_n.png", "start": 283253275, "end": 283253801}, {"filename": "/GameData/textures/lq_palette/flat_10_o.png", "start": 283253801, "end": 283254327}, {"filename": "/GameData/textures/lq_palette/flat_10_p.png", "start": 283254327, "end": 283254853}, {"filename": "/GameData/textures/lq_palette/flat_11_a.png", "start": 283254853, "end": 283255380}, {"filename": "/GameData/textures/lq_palette/flat_11_b.png", "start": 283255380, "end": 283255907}, {"filename": "/GameData/textures/lq_palette/flat_11_c.png", "start": 283255907, "end": 283256434}, {"filename": "/GameData/textures/lq_palette/flat_11_d.png", "start": 283256434, "end": 283256961}, {"filename": "/GameData/textures/lq_palette/flat_11_e.png", "start": 283256961, "end": 283257488}, {"filename": "/GameData/textures/lq_palette/flat_11_f.png", "start": 283257488, "end": 283258014}, {"filename": "/GameData/textures/lq_palette/flat_11_g.png", "start": 283258014, "end": 283258540}, {"filename": "/GameData/textures/lq_palette/flat_11_h.png", "start": 283258540, "end": 283259066}, {"filename": "/GameData/textures/lq_palette/flat_11_i.png", "start": 283259066, "end": 283259592}, {"filename": "/GameData/textures/lq_palette/flat_11_j.png", "start": 283259592, "end": 283260118}, {"filename": "/GameData/textures/lq_palette/flat_11_k.png", "start": 283260118, "end": 283260644}, {"filename": "/GameData/textures/lq_palette/flat_11_l.png", "start": 283260644, "end": 283261170}, {"filename": "/GameData/textures/lq_palette/flat_11_m.png", "start": 283261170, "end": 283261696}, {"filename": "/GameData/textures/lq_palette/flat_11_n.png", "start": 283261696, "end": 283262222}, {"filename": "/GameData/textures/lq_palette/flat_11_o.png", "start": 283262222, "end": 283262748}, {"filename": "/GameData/textures/lq_palette/flat_11_p.png", "start": 283262748, "end": 283263274}, {"filename": "/GameData/textures/lq_palette/flat_12_a.png", "start": 283263274, "end": 283263800}, {"filename": "/GameData/textures/lq_palette/flat_12_b.png", "start": 283263800, "end": 283264326}, {"filename": "/GameData/textures/lq_palette/flat_12_c.png", "start": 283264326, "end": 283264852}, {"filename": "/GameData/textures/lq_palette/flat_12_d.png", "start": 283264852, "end": 283265378}, {"filename": "/GameData/textures/lq_palette/flat_12_e.png", "start": 283265378, "end": 283265904}, {"filename": "/GameData/textures/lq_palette/flat_12_f.png", "start": 283265904, "end": 283266430}, {"filename": "/GameData/textures/lq_palette/flat_12_g.png", "start": 283266430, "end": 283266956}, {"filename": "/GameData/textures/lq_palette/flat_12_h.png", "start": 283266956, "end": 283267482}, {"filename": "/GameData/textures/lq_palette/flat_12_i.png", "start": 283267482, "end": 283268008}, {"filename": "/GameData/textures/lq_palette/flat_12_j.png", "start": 283268008, "end": 283268534}, {"filename": "/GameData/textures/lq_palette/flat_12_k.png", "start": 283268534, "end": 283269060}, {"filename": "/GameData/textures/lq_palette/flat_12_l.png", "start": 283269060, "end": 283269586}, {"filename": "/GameData/textures/lq_palette/flat_12_m.png", "start": 283269586, "end": 283270112}, {"filename": "/GameData/textures/lq_palette/flat_12_n.png", "start": 283270112, "end": 283270638}, {"filename": "/GameData/textures/lq_palette/flat_12_o.png", "start": 283270638, "end": 283271164}, {"filename": "/GameData/textures/lq_palette/flat_12_p.png", "start": 283271164, "end": 283271690}, {"filename": "/GameData/textures/lq_palette/flat_13_a.png", "start": 283271690, "end": 283272216}, {"filename": "/GameData/textures/lq_palette/flat_13_b.png", "start": 283272216, "end": 283272742}, {"filename": "/GameData/textures/lq_palette/flat_13_c.png", "start": 283272742, "end": 283273268}, {"filename": "/GameData/textures/lq_palette/flat_13_d.png", "start": 283273268, "end": 283273794}, {"filename": "/GameData/textures/lq_palette/flat_13_e.png", "start": 283273794, "end": 283274320}, {"filename": "/GameData/textures/lq_palette/flat_13_f.png", "start": 283274320, "end": 283274846}, {"filename": "/GameData/textures/lq_palette/flat_13_g.png", "start": 283274846, "end": 283275372}, {"filename": "/GameData/textures/lq_palette/flat_13_h.png", "start": 283275372, "end": 283275898}, {"filename": "/GameData/textures/lq_palette/flat_13_i.png", "start": 283275898, "end": 283276424}, {"filename": "/GameData/textures/lq_palette/flat_13_j.png", "start": 283276424, "end": 283276950}, {"filename": "/GameData/textures/lq_palette/flat_13_k.png", "start": 283276950, "end": 283277476}, {"filename": "/GameData/textures/lq_palette/flat_13_l.png", "start": 283277476, "end": 283278002}, {"filename": "/GameData/textures/lq_palette/flat_13_m.png", "start": 283278002, "end": 283278528}, {"filename": "/GameData/textures/lq_palette/flat_13_n.png", "start": 283278528, "end": 283279054}, {"filename": "/GameData/textures/lq_palette/flat_13_o.png", "start": 283279054, "end": 283279580}, {"filename": "/GameData/textures/lq_palette/flat_13_p.png", "start": 283279580, "end": 283280106}, {"filename": "/GameData/textures/lq_palette/flat_14_a.png", "start": 283280106, "end": 283280654}, {"filename": "/GameData/textures/lq_palette/flat_14_b.png", "start": 283280654, "end": 283281180}, {"filename": "/GameData/textures/lq_palette/flat_14_c.png", "start": 283281180, "end": 283281706}, {"filename": "/GameData/textures/lq_palette/flat_14_d.png", "start": 283281706, "end": 283282232}, {"filename": "/GameData/textures/lq_palette/flat_14_e.png", "start": 283282232, "end": 283282758}, {"filename": "/GameData/textures/lq_palette/flat_14_f.png", "start": 283282758, "end": 283283284}, {"filename": "/GameData/textures/lq_palette/flat_14_g.png", "start": 283283284, "end": 283283810}, {"filename": "/GameData/textures/lq_palette/flat_14_h.png", "start": 283283810, "end": 283284336}, {"filename": "/GameData/textures/lq_palette/flat_14_i.png", "start": 283284336, "end": 283284862}, {"filename": "/GameData/textures/lq_palette/flat_14_j.png", "start": 283284862, "end": 283285388}, {"filename": "/GameData/textures/lq_palette/flat_14_k.png", "start": 283285388, "end": 283285914}, {"filename": "/GameData/textures/lq_palette/flat_14_l.png", "start": 283285914, "end": 283286440}, {"filename": "/GameData/textures/lq_palette/flat_14_m.png", "start": 283286440, "end": 283286966}, {"filename": "/GameData/textures/lq_palette/flat_14_n.png", "start": 283286966, "end": 283287492}, {"filename": "/GameData/textures/lq_palette/flat_14_o.png", "start": 283287492, "end": 283288018}, {"filename": "/GameData/textures/lq_palette/flat_14_p.png", "start": 283288018, "end": 283288544}, {"filename": "/GameData/textures/lq_palette/flat_15_a_fbr.png", "start": 283288544, "end": 283289070}, {"filename": "/GameData/textures/lq_palette/flat_15_b_fbr.png", "start": 283289070, "end": 283289596}, {"filename": "/GameData/textures/lq_palette/flat_15_c_fbr.png", "start": 283289596, "end": 283290122}, {"filename": "/GameData/textures/lq_palette/flat_15_d_fbr.png", "start": 283290122, "end": 283290648}, {"filename": "/GameData/textures/lq_palette/flat_15_e_fbr.png", "start": 283290648, "end": 283291174}, {"filename": "/GameData/textures/lq_palette/flat_15_f_fbr.png", "start": 283291174, "end": 283291700}, {"filename": "/GameData/textures/lq_palette/flat_15_g_fbr.png", "start": 283291700, "end": 283292226}, {"filename": "/GameData/textures/lq_palette/flat_15_h_fbr.png", "start": 283292226, "end": 283292752}, {"filename": "/GameData/textures/lq_palette/flat_15_i_fbr.png", "start": 283292752, "end": 283293278}, {"filename": "/GameData/textures/lq_palette/flat_15_j_fbr.png", "start": 283293278, "end": 283293804}, {"filename": "/GameData/textures/lq_palette/flat_15_k_fbr.png", "start": 283293804, "end": 283294330}, {"filename": "/GameData/textures/lq_palette/flat_15_l_fbr.png", "start": 283294330, "end": 283294856}, {"filename": "/GameData/textures/lq_palette/flat_15_m_fbr.png", "start": 283294856, "end": 283295382}, {"filename": "/GameData/textures/lq_palette/flat_15_n_fbr.png", "start": 283295382, "end": 283295908}, {"filename": "/GameData/textures/lq_palette/flat_15_o_fbr.png", "start": 283295908, "end": 283296435}, {"filename": "/GameData/textures/lq_palette/flat_15_p_fbr.png", "start": 283296435, "end": 283296962}, {"filename": "/GameData/textures/lq_palette/flat_16_a_fbr.png", "start": 283296962, "end": 283297488}, {"filename": "/GameData/textures/lq_palette/flat_16_b_fbr.png", "start": 283297488, "end": 283298014}, {"filename": "/GameData/textures/lq_palette/flat_16_c_fbr.png", "start": 283298014, "end": 283298540}, {"filename": "/GameData/textures/lq_palette/flat_16_d_fbr.png", "start": 283298540, "end": 283299066}, {"filename": "/GameData/textures/lq_palette/flat_16_e_fbr.png", "start": 283299066, "end": 283299593}, {"filename": "/GameData/textures/lq_palette/flat_16_f_fbr.png", "start": 283299593, "end": 283300120}, {"filename": "/GameData/textures/lq_palette/flat_16_g_fbr.png", "start": 283300120, "end": 283300645}, {"filename": "/GameData/textures/lq_palette/flat_16_h_fbr.png", "start": 283300645, "end": 283301171}, {"filename": "/GameData/textures/lq_palette/flat_16_i_fbr.png", "start": 283301171, "end": 283301697}, {"filename": "/GameData/textures/lq_palette/flat_16_j_fbr.png", "start": 283301697, "end": 283302223}, {"filename": "/GameData/textures/lq_palette/flat_16_k_fbr.png", "start": 283302223, "end": 283302749}, {"filename": "/GameData/textures/lq_palette/flat_16_l_fbr.png", "start": 283302749, "end": 283303275}, {"filename": "/GameData/textures/lq_palette/flat_16_m_fbr.png", "start": 283303275, "end": 283303802}, {"filename": "/GameData/textures/lq_palette/flat_16_n_fbr.png", "start": 283303802, "end": 283304329}, {"filename": "/GameData/textures/lq_palette/flat_16_o_fbr.png", "start": 283304329, "end": 283304854}, {"filename": "/GameData/textures/lq_palette/flat_16_p_fbr.png", "start": 283304854, "end": 283305380}, {"filename": "/GameData/textures/lq_props/JarBod1.png", "start": 283305380, "end": 283305727}, {"filename": "/GameData/textures/lq_props/JarBod2.png", "start": 283305727, "end": 283306045}, {"filename": "/GameData/textures/lq_props/JarTop1.png", "start": 283306045, "end": 283306270}, {"filename": "/GameData/textures/lq_props/JarTop2.png", "start": 283306270, "end": 283306410}, {"filename": "/GameData/textures/lq_props/crate-door-brn.png", "start": 283306410, "end": 283315048}, {"filename": "/GameData/textures/lq_props/crate-door-grn.png", "start": 283315048, "end": 283325104}, {"filename": "/GameData/textures/lq_props/crate-door-orn.png", "start": 283325104, "end": 283334203}, {"filename": "/GameData/textures/lq_props/crate-side-brn.png", "start": 283334203, "end": 283354329}, {"filename": "/GameData/textures/lq_props/crate-side-grn.png", "start": 283354329, "end": 283377242}, {"filename": "/GameData/textures/lq_props/crate-side-orn.png", "start": 283377242, "end": 283398571}, {"filename": "/GameData/textures/lq_props/go-ep0_fbr.png", "start": 283398571, "end": 283400346}, {"filename": "/GameData/textures/lq_props/med_book_blue.png", "start": 283400346, "end": 283400926}, {"filename": "/GameData/textures/lq_props/med_book_green.png", "start": 283400926, "end": 283401549}, {"filename": "/GameData/textures/lq_props/med_book_pink.png", "start": 283401549, "end": 283402204}, {"filename": "/GameData/textures/lq_props/med_book_red.png", "start": 283402204, "end": 283402921}, {"filename": "/GameData/textures/lq_props/med_book_teal.png", "start": 283402921, "end": 283403508}, {"filename": "/GameData/textures/lq_props/med_books_wood.png", "start": 283403508, "end": 283416438}, {"filename": "/GameData/textures/lq_props/med_dbrick4_p1.png", "start": 283416438, "end": 283448275}, {"filename": "/GameData/textures/lq_props/med_dbrick4_p2.png", "start": 283448275, "end": 283481696}, {"filename": "/GameData/textures/lq_props/med_ebrick9_p1.png", "start": 283481696, "end": 283513587}, {"filename": "/GameData/textures/lq_props/med_ebrick9_p2.png", "start": 283513587, "end": 283544953}, {"filename": "/GameData/textures/lq_props/note-e0_fbr.png", "start": 283544953, "end": 283564382}, {"filename": "/GameData/textures/lq_props/plus_0blink_fbr.png", "start": 283564382, "end": 283564611}, {"filename": "/GameData/textures/lq_props/plus_0tvnoise.png", "start": 283564611, "end": 283565597}, {"filename": "/GameData/textures/lq_props/plus_1blink_fbr.png", "start": 283565597, "end": 283565826}, {"filename": "/GameData/textures/lq_props/plus_1tvnoise.png", "start": 283565826, "end": 283566804}, {"filename": "/GameData/textures/lq_props/plus_2blink_fbr.png", "start": 283566804, "end": 283567035}, {"filename": "/GameData/textures/lq_props/plus_2tvnoise.png", "start": 283567035, "end": 283568008}, {"filename": "/GameData/textures/lq_props/plus_3blink_fbr.png", "start": 283568008, "end": 283568239}, {"filename": "/GameData/textures/lq_props/plus_3tvnoise.png", "start": 283568239, "end": 283569221}, {"filename": "/GameData/textures/lq_props/plus_4blink_fbr.png", "start": 283569221, "end": 283569451}, {"filename": "/GameData/textures/lq_props/plus_4tvnoise.png", "start": 283569451, "end": 283570435}, {"filename": "/GameData/textures/lq_props/plus_5tvnoise.png", "start": 283570435, "end": 283571399}, {"filename": "/GameData/textures/lq_props/plus_6tvnoise.png", "start": 283571399, "end": 283572389}, {"filename": "/GameData/textures/lq_props/plus_7tvnoise.png", "start": 283572389, "end": 283573363}, {"filename": "/GameData/textures/lq_props/plus_8tvnoise.png", "start": 283573363, "end": 283574364}, {"filename": "/GameData/textures/lq_props/plus_9tvnoise.png", "start": 283574364, "end": 283575341}, {"filename": "/GameData/textures/lq_props/plus_ablink_fbr.png", "start": 283575341, "end": 283575571}, {"filename": "/GameData/textures/lq_props/plus_atvnoise.png", "start": 283575571, "end": 283576131}, {"filename": "/GameData/textures/lq_props/plus_atvnoise64.png", "start": 283576131, "end": 283577401}, {"filename": "/GameData/textures/lq_props/qr.png", "start": 283577401, "end": 283578821}, {"filename": "/GameData/textures/lq_props/radio16.png", "start": 283578821, "end": 283579589}, {"filename": "/GameData/textures/lq_props/radio32.png", "start": 283579589, "end": 283580579}, {"filename": "/GameData/textures/lq_props/radio64.png", "start": 283580579, "end": 283582337}, {"filename": "/GameData/textures/lq_props/radiowood.png", "start": 283582337, "end": 283584480}, {"filename": "/GameData/textures/lq_props/secret_gem_1.png", "start": 283584480, "end": 283587516}, {"filename": "/GameData/textures/lq_props/secret_gem_2.png", "start": 283587516, "end": 283590220}, {"filename": "/GameData/textures/lq_props/secret_gem_3.png", "start": 283590220, "end": 283593280}, {"filename": "/GameData/textures/lq_props/secret_gem_4.png", "start": 283593280, "end": 283597038}, {"filename": "/GameData/textures/lq_props/secret_gem_h.png", "start": 283597038, "end": 283597589}, {"filename": "/GameData/textures/lq_tech/_t_fence01_fbr.png", "start": 283597589, "end": 283603306}, {"filename": "/GameData/textures/lq_tech/_t_flare01_fbr.png", "start": 283603306, "end": 283603692}, {"filename": "/GameData/textures/lq_tech/aqconc03.png", "start": 283603692, "end": 283614252}, {"filename": "/GameData/textures/lq_tech/aqconc04.png", "start": 283614252, "end": 283625308}, {"filename": "/GameData/textures/lq_tech/aqconc05.png", "start": 283625308, "end": 283629899}, {"filename": "/GameData/textures/lq_tech/aqf006b.png", "start": 283629899, "end": 283632526}, {"filename": "/GameData/textures/lq_tech/aqf032.png", "start": 283632526, "end": 283633797}, {"filename": "/GameData/textures/lq_tech/aqf049.png", "start": 283633797, "end": 283636074}, {"filename": "/GameData/textures/lq_tech/aqf074.png", "start": 283636074, "end": 283639496}, {"filename": "/GameData/textures/lq_tech/aqf075.png", "start": 283639496, "end": 283642655}, {"filename": "/GameData/textures/lq_tech/aqmetl01.png", "start": 283642655, "end": 283645383}, {"filename": "/GameData/textures/lq_tech/aqmetl07.png", "start": 283645383, "end": 283650154}, {"filename": "/GameData/textures/lq_tech/aqmetl14.png", "start": 283650154, "end": 283657886}, {"filename": "/GameData/textures/lq_tech/aqmetl28.png", "start": 283657886, "end": 283659538}, {"filename": "/GameData/textures/lq_tech/aqmetl30.png", "start": 283659538, "end": 283661190}, {"filename": "/GameData/textures/lq_tech/aqmetl33.png", "start": 283661190, "end": 283663626}, {"filename": "/GameData/textures/lq_tech/aqpanl09.png", "start": 283663626, "end": 283666418}, {"filename": "/GameData/textures/lq_tech/aqpanl10.png", "start": 283666418, "end": 283671312}, {"filename": "/GameData/textures/lq_tech/aqpipe01.png", "start": 283671312, "end": 283675500}, {"filename": "/GameData/textures/lq_tech/aqpipe04.png", "start": 283675500, "end": 283677964}, {"filename": "/GameData/textures/lq_tech/aqpipe05.png", "start": 283677964, "end": 283681130}, {"filename": "/GameData/textures/lq_tech/aqpipe08.png", "start": 283681130, "end": 283686762}, {"filename": "/GameData/textures/lq_tech/aqpipe09.png", "start": 283686762, "end": 283694866}, {"filename": "/GameData/textures/lq_tech/aqpipe12.png", "start": 283694866, "end": 283702986}, {"filename": "/GameData/textures/lq_tech/aqpipe13.png", "start": 283702986, "end": 283709297}, {"filename": "/GameData/textures/lq_tech/aqpipe14.png", "start": 283709297, "end": 283716650}, {"filename": "/GameData/textures/lq_tech/aqrust01.png", "start": 283716650, "end": 283719281}, {"filename": "/GameData/textures/lq_tech/aqrust02.png", "start": 283719281, "end": 283724582}, {"filename": "/GameData/textures/lq_tech/aqrust03.png", "start": 283724582, "end": 283729885}, {"filename": "/GameData/textures/lq_tech/aqrust03b.png", "start": 283729885, "end": 283732615}, {"filename": "/GameData/textures/lq_tech/aqrust04.png", "start": 283732615, "end": 283734008}, {"filename": "/GameData/textures/lq_tech/aqrust09.png", "start": 283734008, "end": 283737896}, {"filename": "/GameData/textures/lq_tech/aqrust10.png", "start": 283737896, "end": 283741062}, {"filename": "/GameData/textures/lq_tech/aqsect14.png", "start": 283741062, "end": 283743438}, {"filename": "/GameData/textures/lq_tech/aqsect15.png", "start": 283743438, "end": 283746385}, {"filename": "/GameData/textures/lq_tech/aqsect16.png", "start": 283746385, "end": 283749631}, {"filename": "/GameData/textures/lq_tech/aqsect16b.png", "start": 283749631, "end": 283754588}, {"filename": "/GameData/textures/lq_tech/aqsupp01.png", "start": 283754588, "end": 283757234}, {"filename": "/GameData/textures/lq_tech/aqsupp02.png", "start": 283757234, "end": 283762099}, {"filename": "/GameData/textures/lq_tech/aqsupp03.png", "start": 283762099, "end": 283766001}, {"filename": "/GameData/textures/lq_tech/aqsupp04.png", "start": 283766001, "end": 283767544}, {"filename": "/GameData/textures/lq_tech/aqsupp06.png", "start": 283767544, "end": 283768923}, {"filename": "/GameData/textures/lq_tech/aqsupp07.png", "start": 283768923, "end": 283770278}, {"filename": "/GameData/textures/lq_tech/aqsupp08.png", "start": 283770278, "end": 283772480}, {"filename": "/GameData/textures/lq_tech/aqsupp09.png", "start": 283772480, "end": 283775333}, {"filename": "/GameData/textures/lq_tech/aqtrim01.png", "start": 283775333, "end": 283776560}, {"filename": "/GameData/textures/lq_tech/aqtrim02.png", "start": 283776560, "end": 283777484}, {"filename": "/GameData/textures/lq_tech/aqtrim03.png", "start": 283777484, "end": 283778055}, {"filename": "/GameData/textures/lq_tech/aqtrim08.png", "start": 283778055, "end": 283778787}, {"filename": "/GameData/textures/lq_tech/butmet.png", "start": 283778787, "end": 283780698}, {"filename": "/GameData/textures/lq_tech/comp1_1.png", "start": 283780698, "end": 283784054}, {"filename": "/GameData/textures/lq_tech/comp1_2.png", "start": 283784054, "end": 283786733}, {"filename": "/GameData/textures/lq_tech/comp1_3.png", "start": 283786733, "end": 283789222}, {"filename": "/GameData/textures/lq_tech/comp1_3b.png", "start": 283789222, "end": 283791850}, {"filename": "/GameData/textures/lq_tech/comp1_4.png", "start": 283791850, "end": 283793455}, {"filename": "/GameData/textures/lq_tech/comp1_5.png", "start": 283793455, "end": 283796554}, {"filename": "/GameData/textures/lq_tech/comp1_6.png", "start": 283796554, "end": 283799996}, {"filename": "/GameData/textures/lq_tech/comp1_7.png", "start": 283799996, "end": 283802341}, {"filename": "/GameData/textures/lq_tech/comp1_8.png", "start": 283802341, "end": 283804670}, {"filename": "/GameData/textures/lq_tech/compbase.png", "start": 283804670, "end": 283807089}, {"filename": "/GameData/textures/lq_tech/crate.png", "start": 283807089, "end": 283810206}, {"filename": "/GameData/textures/lq_tech/crate0_bottom.png", "start": 283810206, "end": 283812322}, {"filename": "/GameData/textures/lq_tech/crate0_s_bottom.png", "start": 283812322, "end": 283813410}, {"filename": "/GameData/textures/lq_tech/crate0_s_sside.png", "start": 283813410, "end": 283814355}, {"filename": "/GameData/textures/lq_tech/crate0_s_top.png", "start": 283814355, "end": 283815794}, {"filename": "/GameData/textures/lq_tech/crate0_s_tside.png", "start": 283815794, "end": 283817457}, {"filename": "/GameData/textures/lq_tech/crate0_side.png", "start": 283817457, "end": 283820925}, {"filename": "/GameData/textures/lq_tech/crate0_top.png", "start": 283820925, "end": 283823671}, {"filename": "/GameData/textures/lq_tech/crate0_xs_bot.png", "start": 283823671, "end": 283824338}, {"filename": "/GameData/textures/lq_tech/crate0_xs_sside.png", "start": 283824338, "end": 283825283}, {"filename": "/GameData/textures/lq_tech/crate0_xs_top.png", "start": 283825283, "end": 283826153}, {"filename": "/GameData/textures/lq_tech/crate0_xs_tside.png", "start": 283826153, "end": 283827093}, {"filename": "/GameData/textures/lq_tech/crate1_bottom.png", "start": 283827093, "end": 283829057}, {"filename": "/GameData/textures/lq_tech/crate1_s_bottom.png", "start": 283829057, "end": 283830087}, {"filename": "/GameData/textures/lq_tech/crate1_s_sside.png", "start": 283830087, "end": 283830965}, {"filename": "/GameData/textures/lq_tech/crate1_s_top.png", "start": 283830965, "end": 283832098}, {"filename": "/GameData/textures/lq_tech/crate1_s_tside.png", "start": 283832098, "end": 283833563}, {"filename": "/GameData/textures/lq_tech/crate1_side.png", "start": 283833563, "end": 283836680}, {"filename": "/GameData/textures/lq_tech/crate1_top.png", "start": 283836680, "end": 283838801}, {"filename": "/GameData/textures/lq_tech/crate1_xs_bot.png", "start": 283838801, "end": 283839455}, {"filename": "/GameData/textures/lq_tech/crate1_xs_sside.png", "start": 283839455, "end": 283840333}, {"filename": "/GameData/textures/lq_tech/crate1_xs_top.png", "start": 283840333, "end": 283841040}, {"filename": "/GameData/textures/lq_tech/crate1_xs_tside.png", "start": 283841040, "end": 283841878}, {"filename": "/GameData/textures/lq_tech/dem4_1.png", "start": 283841878, "end": 283851277}, {"filename": "/GameData/textures/lq_tech/dem4_4.png", "start": 283851277, "end": 283859323}, {"filename": "/GameData/textures/lq_tech/dem5_3_fbr.png", "start": 283859323, "end": 283868140}, {"filename": "/GameData/textures/lq_tech/door02_1.png", "start": 283868140, "end": 283872272}, {"filename": "/GameData/textures/lq_tech/doorr02_1.png", "start": 283872272, "end": 283873838}, {"filename": "/GameData/textures/lq_tech/doortrak1.png", "start": 283873838, "end": 283875449}, {"filename": "/GameData/textures/lq_tech/doortrak2-corn.png", "start": 283875449, "end": 283876746}, {"filename": "/GameData/textures/lq_tech/doortrak2.png", "start": 283876746, "end": 283878411}, {"filename": "/GameData/textures/lq_tech/ecop1_1.png", "start": 283878411, "end": 283881488}, {"filename": "/GameData/textures/lq_tech/ecop1_4.png", "start": 283881488, "end": 283885089}, {"filename": "/GameData/textures/lq_tech/edoor01_1.png", "start": 283885089, "end": 283897235}, {"filename": "/GameData/textures/lq_tech/edoor02.png", "start": 283897235, "end": 283909286}, {"filename": "/GameData/textures/lq_tech/edoor02.png.png", "start": 283909286, "end": 283921337}, {"filename": "/GameData/textures/lq_tech/fddoor01.png", "start": 283921337, "end": 283930091}, {"filename": "/GameData/textures/lq_tech/fddoor01b.png", "start": 283930091, "end": 283940774}, {"filename": "/GameData/textures/lq_tech/fdoor02.png", "start": 283940774, "end": 283949328}, {"filename": "/GameData/textures/lq_tech/flat4.png", "start": 283949328, "end": 283950501}, {"filename": "/GameData/textures/lq_tech/floor5_2.png", "start": 283950501, "end": 283953227}, {"filename": "/GameData/textures/lq_tech/floor5_3.png", "start": 283953227, "end": 283955407}, {"filename": "/GameData/textures/lq_tech/laserfield1_fbr.png", "start": 283955407, "end": 283966416}, {"filename": "/GameData/textures/lq_tech/light2.png", "start": 283966416, "end": 283967014}, {"filename": "/GameData/textures/lq_tech/lit8nb.png", "start": 283967014, "end": 283967328}, {"filename": "/GameData/textures/lq_tech/lit8sfb_fbr.png", "start": 283967328, "end": 283967625}, {"filename": "/GameData/textures/lq_tech/met2.png", "start": 283967625, "end": 283979949}, {"filename": "/GameData/textures/lq_tech/metalstrip_1.png", "start": 283979949, "end": 283982232}, {"filename": "/GameData/textures/lq_tech/plat_side1.png", "start": 283982232, "end": 283982997}, {"filename": "/GameData/textures/lq_tech/plat_stem.png", "start": 283982997, "end": 283983643}, {"filename": "/GameData/textures/lq_tech/plat_top1.png", "start": 283983643, "end": 283987043}, {"filename": "/GameData/textures/lq_tech/plat_top2.png", "start": 283987043, "end": 283990758}, {"filename": "/GameData/textures/lq_tech/plat_top3.png", "start": 283990758, "end": 283994166}, {"filename": "/GameData/textures/lq_tech/plat_top4.png", "start": 283994166, "end": 283996862}, {"filename": "/GameData/textures/lq_tech/plat_top5.png", "start": 283996862, "end": 283998957}, {"filename": "/GameData/textures/lq_tech/plus_0_gkey.png", "start": 283998957, "end": 284000021}, {"filename": "/GameData/textures/lq_tech/plus_0_skey.png", "start": 284000021, "end": 284001037}, {"filename": "/GameData/textures/lq_tech/plus_0_tscrn0.png", "start": 284001037, "end": 284003150}, {"filename": "/GameData/textures/lq_tech/plus_0_tscrn1.png", "start": 284003150, "end": 284005275}, {"filename": "/GameData/textures/lq_tech/plus_0arrow2_d_fbr.png", "start": 284005275, "end": 284006529}, {"filename": "/GameData/textures/lq_tech/plus_0arrow2_h_fbr.png", "start": 284006529, "end": 284007798}, {"filename": "/GameData/textures/lq_tech/plus_0arrow2_u_fbr.png", "start": 284007798, "end": 284009056}, {"filename": "/GameData/textures/lq_tech/plus_0arrow_d_fbr.png", "start": 284009056, "end": 284009938}, {"filename": "/GameData/textures/lq_tech/plus_0arrow_h_fbr.png", "start": 284009938, "end": 284010817}, {"filename": "/GameData/textures/lq_tech/plus_0arrow_u_fbr.png", "start": 284010817, "end": 284011692}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn1b_fbr.png", "start": 284011692, "end": 284012581}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn2_fbr.png", "start": 284012581, "end": 284012927}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn2b_fbr.png", "start": 284012927, "end": 284013267}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn_fbr.png", "start": 284013267, "end": 284014548}, {"filename": "/GameData/textures/lq_tech/plus_0button3_fbr.png", "start": 284014548, "end": 284016424}, {"filename": "/GameData/textures/lq_tech/plus_0lit8s.png", "start": 284016424, "end": 284016738}, {"filename": "/GameData/textures/lq_tech/plus_0planet_a_fbr.png", "start": 284016738, "end": 284018329}, {"filename": "/GameData/textures/lq_tech/plus_0planet_b_fbr.png", "start": 284018329, "end": 284019439}, {"filename": "/GameData/textures/lq_tech/plus_0planet_c_fbr.png", "start": 284019439, "end": 284020539}, {"filename": "/GameData/textures/lq_tech/plus_0slipbot.png", "start": 284020539, "end": 284023822}, {"filename": "/GameData/textures/lq_tech/plus_0sliptop.png", "start": 284023822, "end": 284027465}, {"filename": "/GameData/textures/lq_tech/plus_0tek_jump1_fbr.png", "start": 284027465, "end": 284029948}, {"filename": "/GameData/textures/lq_tech/plus_0term128.png", "start": 284029948, "end": 284032752}, {"filename": "/GameData/textures/lq_tech/plus_0term64.png", "start": 284032752, "end": 284033944}, {"filename": "/GameData/textures/lq_tech/plus_0tlight1.png", "start": 284033944, "end": 284034542}, {"filename": "/GameData/textures/lq_tech/plus_0tlight2.png", "start": 284034542, "end": 284035155}, {"filename": "/GameData/textures/lq_tech/plus_0tlight3.png", "start": 284035155, "end": 284035726}, {"filename": "/GameData/textures/lq_tech/plus_1_gkey.png", "start": 284035726, "end": 284036776}, {"filename": "/GameData/textures/lq_tech/plus_1_skey.png", "start": 284036776, "end": 284037781}, {"filename": "/GameData/textures/lq_tech/plus_1arrow2_d_fbr.png", "start": 284037781, "end": 284039037}, {"filename": "/GameData/textures/lq_tech/plus_1arrow2_h_fbr.png", "start": 284039037, "end": 284040304}, {"filename": "/GameData/textures/lq_tech/plus_1arrow2_u_fbr.png", "start": 284040304, "end": 284041562}, {"filename": "/GameData/textures/lq_tech/plus_1arrow_d_fbr.png", "start": 284041562, "end": 284042448}, {"filename": "/GameData/textures/lq_tech/plus_1arrow_h_fbr.png", "start": 284042448, "end": 284043330}, {"filename": "/GameData/textures/lq_tech/plus_1arrow_u_fbr.png", "start": 284043330, "end": 284044208}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn.png", "start": 284044208, "end": 284045487}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn1b.png", "start": 284045487, "end": 284046365}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn2.png", "start": 284046365, "end": 284046715}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn2b.png", "start": 284046715, "end": 284047073}, {"filename": "/GameData/textures/lq_tech/plus_1planet_a_fbr.png", "start": 284047073, "end": 284048694}, {"filename": "/GameData/textures/lq_tech/plus_1planet_b_fbr.png", "start": 284048694, "end": 284049779}, {"filename": "/GameData/textures/lq_tech/plus_1planet_c_fbr.png", "start": 284049779, "end": 284050851}, {"filename": "/GameData/textures/lq_tech/plus_1tek_jump1_fbr.png", "start": 284050851, "end": 284053334}, {"filename": "/GameData/textures/lq_tech/plus_1term128.png", "start": 284053334, "end": 284056137}, {"filename": "/GameData/textures/lq_tech/plus_1term64.png", "start": 284056137, "end": 284057329}, {"filename": "/GameData/textures/lq_tech/plus_2_gkey.png", "start": 284057329, "end": 284058391}, {"filename": "/GameData/textures/lq_tech/plus_2_skey.png", "start": 284058391, "end": 284059397}, {"filename": "/GameData/textures/lq_tech/plus_2arrow2_d_fbr.png", "start": 284059397, "end": 284060640}, {"filename": "/GameData/textures/lq_tech/plus_2arrow2_h_fbr.png", "start": 284060640, "end": 284061901}, {"filename": "/GameData/textures/lq_tech/plus_2arrow2_u_fbr.png", "start": 284061901, "end": 284063147}, {"filename": "/GameData/textures/lq_tech/plus_2arrow_d_fbr.png", "start": 284063147, "end": 284064045}, {"filename": "/GameData/textures/lq_tech/plus_2arrow_h_fbr.png", "start": 284064045, "end": 284064935}, {"filename": "/GameData/textures/lq_tech/plus_2arrow_u_fbr.png", "start": 284064935, "end": 284065822}, {"filename": "/GameData/textures/lq_tech/plus_2planet_a_fbr.png", "start": 284065822, "end": 284067425}, {"filename": "/GameData/textures/lq_tech/plus_2planet_b_fbr.png", "start": 284067425, "end": 284068527}, {"filename": "/GameData/textures/lq_tech/plus_2planet_c_fbr.png", "start": 284068527, "end": 284069645}, {"filename": "/GameData/textures/lq_tech/plus_3planet_a_fbr.png", "start": 284069645, "end": 284071235}, {"filename": "/GameData/textures/lq_tech/plus_3planet_b_fbr.png", "start": 284071235, "end": 284072329}, {"filename": "/GameData/textures/lq_tech/plus_3planet_c_fbr.png", "start": 284072329, "end": 284073452}, {"filename": "/GameData/textures/lq_tech/plus_4planet_a_fbr.png", "start": 284073452, "end": 284075061}, {"filename": "/GameData/textures/lq_tech/plus_4planet_b_fbr.png", "start": 284075061, "end": 284076176}, {"filename": "/GameData/textures/lq_tech/plus_4planet_c_fbr.png", "start": 284076176, "end": 284077323}, {"filename": "/GameData/textures/lq_tech/plus_5planet_a_fbr.png", "start": 284077323, "end": 284078955}, {"filename": "/GameData/textures/lq_tech/plus_5planet_b_fbr.png", "start": 284078955, "end": 284080082}, {"filename": "/GameData/textures/lq_tech/plus_5planet_c_fbr.png", "start": 284080082, "end": 284081189}, {"filename": "/GameData/textures/lq_tech/plus_6planet_a_fbr.png", "start": 284081189, "end": 284082810}, {"filename": "/GameData/textures/lq_tech/plus_6planet_b_fbr.png", "start": 284082810, "end": 284083934}, {"filename": "/GameData/textures/lq_tech/plus_6planet_c_fbr.png", "start": 284083934, "end": 284085028}, {"filename": "/GameData/textures/lq_tech/plus_7planet_a_fbr.png", "start": 284085028, "end": 284086616}, {"filename": "/GameData/textures/lq_tech/plus_7planet_b_fbr.png", "start": 284086616, "end": 284087729}, {"filename": "/GameData/textures/lq_tech/plus_7planet_c_fbr.png", "start": 284087729, "end": 284088821}, {"filename": "/GameData/textures/lq_tech/plus_8planet_a_fbr.png", "start": 284088821, "end": 284090443}, {"filename": "/GameData/textures/lq_tech/plus_8planet_b_fbr.png", "start": 284090443, "end": 284091570}, {"filename": "/GameData/textures/lq_tech/plus_8planet_c_fbr.png", "start": 284091570, "end": 284092659}, {"filename": "/GameData/textures/lq_tech/plus_9planet_a_fbr.png", "start": 284092659, "end": 284094243}, {"filename": "/GameData/textures/lq_tech/plus_9planet_b_fbr.png", "start": 284094243, "end": 284095331}, {"filename": "/GameData/textures/lq_tech/plus_9planet_c_fbr.png", "start": 284095331, "end": 284096407}, {"filename": "/GameData/textures/lq_tech/plus_A_tscrn0.png", "start": 284096407, "end": 284098030}, {"filename": "/GameData/textures/lq_tech/plus_A_tscrn1.png", "start": 284098030, "end": 284100539}, {"filename": "/GameData/textures/lq_tech/plus_A_tscrn2.png", "start": 284100539, "end": 284102139}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn.png", "start": 284102139, "end": 284103428}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn1b.png", "start": 284103428, "end": 284104717}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn2.png", "start": 284104717, "end": 284105088}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn2b.png", "start": 284105088, "end": 284105459}, {"filename": "/GameData/textures/lq_tech/plus_abasebtnb.png", "start": 284105459, "end": 284105830}, {"filename": "/GameData/textures/lq_tech/plus_abutton3_fbr.png", "start": 284105830, "end": 284107718}, {"filename": "/GameData/textures/lq_tech/plus_alit8s_fbr.png", "start": 284107718, "end": 284108015}, {"filename": "/GameData/textures/lq_tech/plus_atek_jump1_fbr.png", "start": 284108015, "end": 284110508}, {"filename": "/GameData/textures/lq_tech/plus_atlight1_fbr.png", "start": 284110508, "end": 284111098}, {"filename": "/GameData/textures/lq_tech/plus_atlight2_fbr.png", "start": 284111098, "end": 284111671}, {"filename": "/GameData/textures/lq_tech/plus_atlight3_fbr.png", "start": 284111671, "end": 284112273}, {"filename": "/GameData/textures/lq_tech/rw33_1.png", "start": 284112273, "end": 284117838}, {"filename": "/GameData/textures/lq_tech/rw33_2.png", "start": 284117838, "end": 284123692}, {"filename": "/GameData/textures/lq_tech/rw33_3.png", "start": 284123692, "end": 284129438}, {"filename": "/GameData/textures/lq_tech/rw33_4.png", "start": 284129438, "end": 284132004}, {"filename": "/GameData/textures/lq_tech/rw33_4b_l.png", "start": 284132004, "end": 284134964}, {"filename": "/GameData/textures/lq_tech/rw33_5.png", "start": 284134964, "end": 284137836}, {"filename": "/GameData/textures/lq_tech/rw33_flat.png", "start": 284137836, "end": 284143787}, {"filename": "/GameData/textures/lq_tech/rw33_lit.png", "start": 284143787, "end": 284144611}, {"filename": "/GameData/textures/lq_tech/rw33b_1.png", "start": 284144611, "end": 284150074}, {"filename": "/GameData/textures/lq_tech/rw33b_2.png", "start": 284150074, "end": 284155916}, {"filename": "/GameData/textures/lq_tech/rw33b_3.png", "start": 284155916, "end": 284161903}, {"filename": "/GameData/textures/lq_tech/rw33b_4.png", "start": 284161903, "end": 284164665}, {"filename": "/GameData/textures/lq_tech/rw33b_5.png", "start": 284164665, "end": 284167714}, {"filename": "/GameData/textures/lq_tech/rw33b_flat.png", "start": 284167714, "end": 284173912}, {"filename": "/GameData/textures/lq_tech/rw33b_lit.png", "start": 284173912, "end": 284174864}, {"filename": "/GameData/textures/lq_tech/rw37_1.png", "start": 284174864, "end": 284180476}, {"filename": "/GameData/textures/lq_tech/rw37_2.png", "start": 284180476, "end": 284186606}, {"filename": "/GameData/textures/lq_tech/rw37_3.png", "start": 284186606, "end": 284192501}, {"filename": "/GameData/textures/lq_tech/rw37_4.png", "start": 284192501, "end": 284199512}, {"filename": "/GameData/textures/lq_tech/rw37_trim1.png", "start": 284199512, "end": 284202020}, {"filename": "/GameData/textures/lq_tech/rw37_trim2.png", "start": 284202020, "end": 284203803}, {"filename": "/GameData/textures/lq_tech/rw37_trim3.png", "start": 284203803, "end": 284206294}, {"filename": "/GameData/textures/lq_tech/rw39_1_fbr.png", "start": 284206294, "end": 284212239}, {"filename": "/GameData/textures/lq_tech/spotlight_fbr.png", "start": 284212239, "end": 284214851}, {"filename": "/GameData/textures/lq_tech/star_lasergrid.png", "start": 284214851, "end": 284215231}, {"filename": "/GameData/textures/lq_tech/t_band1a.png", "start": 284215231, "end": 284217735}, {"filename": "/GameData/textures/lq_tech/t_band1b.png", "start": 284217735, "end": 284220343}, {"filename": "/GameData/textures/lq_tech/t_blok01.png", "start": 284220343, "end": 284223021}, {"filename": "/GameData/textures/lq_tech/t_blok01a.png", "start": 284223021, "end": 284225793}, {"filename": "/GameData/textures/lq_tech/t_blok02.png", "start": 284225793, "end": 284230920}, {"filename": "/GameData/textures/lq_tech/t_blok02a.png", "start": 284230920, "end": 284235779}, {"filename": "/GameData/textures/lq_tech/t_blok03.png", "start": 284235779, "end": 284238221}, {"filename": "/GameData/textures/lq_tech/t_blok03a.png", "start": 284238221, "end": 284240076}, {"filename": "/GameData/textures/lq_tech/t_blok04.png", "start": 284240076, "end": 284243142}, {"filename": "/GameData/textures/lq_tech/t_blok04h.png", "start": 284243142, "end": 284245866}, {"filename": "/GameData/textures/lq_tech/t_blok05.png", "start": 284245866, "end": 284250154}, {"filename": "/GameData/textures/lq_tech/t_blok06.png", "start": 284250154, "end": 284253004}, {"filename": "/GameData/textures/lq_tech/t_blok06h.png", "start": 284253004, "end": 284254981}, {"filename": "/GameData/textures/lq_tech/t_blok07.png", "start": 284254981, "end": 284257767}, {"filename": "/GameData/textures/lq_tech/t_blok07a.png", "start": 284257767, "end": 284260553}, {"filename": "/GameData/textures/lq_tech/t_blok08.png", "start": 284260553, "end": 284265060}, {"filename": "/GameData/textures/lq_tech/t_blok09.png", "start": 284265060, "end": 284268380}, {"filename": "/GameData/textures/lq_tech/t_blok10.png", "start": 284268380, "end": 284274391}, {"filename": "/GameData/textures/lq_tech/t_blok10b.png", "start": 284274391, "end": 284280099}, {"filename": "/GameData/textures/lq_tech/t_blok10c.png", "start": 284280099, "end": 284283317}, {"filename": "/GameData/textures/lq_tech/t_blok11.png", "start": 284283317, "end": 284289258}, {"filename": "/GameData/textures/lq_tech/t_blok11b.png", "start": 284289258, "end": 284294863}, {"filename": "/GameData/textures/lq_tech/t_blok12c.png", "start": 284294863, "end": 284298070}, {"filename": "/GameData/textures/lq_tech/t_flat01.png", "start": 284298070, "end": 284300504}, {"filename": "/GameData/textures/lq_tech/t_flat02.png", "start": 284300504, "end": 284302869}, {"filename": "/GameData/textures/lq_tech/t_flat05.png", "start": 284302869, "end": 284311065}, {"filename": "/GameData/textures/lq_tech/t_flor1a.png", "start": 284311065, "end": 284314677}, {"filename": "/GameData/textures/lq_tech/t_flor1b.png", "start": 284314677, "end": 284318295}, {"filename": "/GameData/textures/lq_tech/t_flor2a.png", "start": 284318295, "end": 284320889}, {"filename": "/GameData/textures/lq_tech/t_flor2b.png", "start": 284320889, "end": 284322825}, {"filename": "/GameData/textures/lq_tech/t_flor2c.png", "start": 284322825, "end": 284324305}, {"filename": "/GameData/textures/lq_tech/t_flor2d.png", "start": 284324305, "end": 284324809}, {"filename": "/GameData/textures/lq_tech/t_lit01_fbr.png", "start": 284324809, "end": 284324989}, {"filename": "/GameData/textures/lq_tech/t_lit02_fbr.png", "start": 284324989, "end": 284325131}, {"filename": "/GameData/textures/lq_tech/t_lit03_fbr.png", "start": 284325131, "end": 284325266}, {"filename": "/GameData/textures/lq_tech/t_lit04_fbr.png", "start": 284325266, "end": 284325398}, {"filename": "/GameData/textures/lq_tech/t_lit05_fbr.png", "start": 284325398, "end": 284325564}, {"filename": "/GameData/textures/lq_tech/t_lit06_fbr.png", "start": 284325564, "end": 284325720}, {"filename": "/GameData/textures/lq_tech/t_lit07_fbr.png", "start": 284325720, "end": 284328023}, {"filename": "/GameData/textures/lq_tech/t_lit08_fbr.png", "start": 284328023, "end": 284328240}, {"filename": "/GameData/textures/lq_tech/t_metalsheeta.png", "start": 284328240, "end": 284334839}, {"filename": "/GameData/textures/lq_tech/t_metalsheetb.png", "start": 284334839, "end": 284346352}, {"filename": "/GameData/textures/lq_tech/t_num_0_fbr.png", "start": 284346352, "end": 284347126}, {"filename": "/GameData/textures/lq_tech/t_num_1_fbr.png", "start": 284347126, "end": 284347888}, {"filename": "/GameData/textures/lq_tech/t_num_2_fbr.png", "start": 284347888, "end": 284348683}, {"filename": "/GameData/textures/lq_tech/t_num_3_fbr.png", "start": 284348683, "end": 284349441}, {"filename": "/GameData/textures/lq_tech/t_num_4_fbr.png", "start": 284349441, "end": 284350246}, {"filename": "/GameData/textures/lq_tech/t_num_5_fbr.png", "start": 284350246, "end": 284351044}, {"filename": "/GameData/textures/lq_tech/t_num_6_fbr.png", "start": 284351044, "end": 284351841}, {"filename": "/GameData/textures/lq_tech/t_num_7_fbr.png", "start": 284351841, "end": 284352615}, {"filename": "/GameData/textures/lq_tech/t_num_8_fbr.png", "start": 284352615, "end": 284353395}, {"filename": "/GameData/textures/lq_tech/t_num_9_fbr.png", "start": 284353395, "end": 284354188}, {"filename": "/GameData/textures/lq_tech/t_num_x.png", "start": 284354188, "end": 284354996}, {"filename": "/GameData/textures/lq_tech/t_rivs01.png", "start": 284354996, "end": 284357424}, {"filename": "/GameData/textures/lq_tech/t_rivs01a.png", "start": 284357424, "end": 284359878}, {"filename": "/GameData/textures/lq_tech/t_sign1.png", "start": 284359878, "end": 284362828}, {"filename": "/GameData/textures/lq_tech/t_tech01.png", "start": 284362828, "end": 284366879}, {"filename": "/GameData/textures/lq_tech/t_tech02.png", "start": 284366879, "end": 284369982}, {"filename": "/GameData/textures/lq_tech/t_tech03.png", "start": 284369982, "end": 284385973}, {"filename": "/GameData/textures/lq_tech/t_tech04.png", "start": 284385973, "end": 284388828}, {"filename": "/GameData/textures/lq_tech/t_tech05.png", "start": 284388828, "end": 284391607}, {"filename": "/GameData/textures/lq_tech/t_tech06.png", "start": 284391607, "end": 284394419}, {"filename": "/GameData/textures/lq_tech/t_trim1a.png", "start": 284394419, "end": 284396907}, {"filename": "/GameData/textures/lq_tech/t_trim1aa.png", "start": 284396907, "end": 284399542}, {"filename": "/GameData/textures/lq_tech/t_trim1b.png", "start": 284399542, "end": 284402273}, {"filename": "/GameData/textures/lq_tech/t_trim1ba.png", "start": 284402273, "end": 284404897}, {"filename": "/GameData/textures/lq_tech/t_trim1c.png", "start": 284404897, "end": 284407682}, {"filename": "/GameData/textures/lq_tech/t_trim1ca.png", "start": 284407682, "end": 284410508}, {"filename": "/GameData/textures/lq_tech/t_trim1d.png", "start": 284410508, "end": 284412808}, {"filename": "/GameData/textures/lq_tech/t_trim1e.png", "start": 284412808, "end": 284414953}, {"filename": "/GameData/textures/lq_tech/t_trim2a.png", "start": 284414953, "end": 284418026}, {"filename": "/GameData/textures/lq_tech/t_trim2aa.png", "start": 284418026, "end": 284420678}, {"filename": "/GameData/textures/lq_tech/t_trim2b.png", "start": 284420678, "end": 284423306}, {"filename": "/GameData/textures/lq_tech/t_trim2ba.png", "start": 284423306, "end": 284425994}, {"filename": "/GameData/textures/lq_tech/t_trim2c.png", "start": 284425994, "end": 284428785}, {"filename": "/GameData/textures/lq_tech/t_trim2ca.png", "start": 284428785, "end": 284431664}, {"filename": "/GameData/textures/lq_tech/t_trim2d.png", "start": 284431664, "end": 284434126}, {"filename": "/GameData/textures/lq_tech/t_trim2e.png", "start": 284434126, "end": 284436633}, {"filename": "/GameData/textures/lq_tech/t_tris02.png", "start": 284436633, "end": 284439547}, {"filename": "/GameData/textures/lq_tech/t_wall05.png", "start": 284439547, "end": 284442873}, {"filename": "/GameData/textures/lq_tech/t_wall1a.png", "start": 284442873, "end": 284453400}, {"filename": "/GameData/textures/lq_tech/t_wall1aa.png", "start": 284453400, "end": 284463721}, {"filename": "/GameData/textures/lq_tech/t_wall1b.png", "start": 284463721, "end": 284474821}, {"filename": "/GameData/textures/lq_tech/t_wall1ba.png", "start": 284474821, "end": 284485660}, {"filename": "/GameData/textures/lq_tech/t_wall2a.png", "start": 284485660, "end": 284496444}, {"filename": "/GameData/textures/lq_tech/t_wall2aa.png", "start": 284496444, "end": 284507464}, {"filename": "/GameData/textures/lq_tech/t_wall2ab.png", "start": 284507464, "end": 284519221}, {"filename": "/GameData/textures/lq_tech/t_wall2b.png", "start": 284519221, "end": 284533459}, {"filename": "/GameData/textures/lq_tech/t_wall2ba.png", "start": 284533459, "end": 284546910}, {"filename": "/GameData/textures/lq_tech/t_wall3a.png", "start": 284546910, "end": 284554877}, {"filename": "/GameData/textures/lq_tech/t_wall3aa.png", "start": 284554877, "end": 284563144}, {"filename": "/GameData/textures/lq_tech/t_wall3b.png", "start": 284563144, "end": 284571466}, {"filename": "/GameData/textures/lq_tech/t_wall3ba.png", "start": 284571466, "end": 284580429}, {"filename": "/GameData/textures/lq_tech/t_wall6a.png", "start": 284580429, "end": 284583663}, {"filename": "/GameData/textures/lq_tech/t_wall6b.png", "start": 284583663, "end": 284586677}, {"filename": "/GameData/textures/lq_tech/t_wall6c.png", "start": 284586677, "end": 284590353}, {"filename": "/GameData/textures/lq_tech/t_wall6d.png", "start": 284590353, "end": 284594056}, {"filename": "/GameData/textures/lq_tech/t_wall6e.png", "start": 284594056, "end": 284597852}, {"filename": "/GameData/textures/lq_tech/t_wall7a.png", "start": 284597852, "end": 284608744}, {"filename": "/GameData/textures/lq_tech/t_wall7b.png", "start": 284608744, "end": 284616185}, {"filename": "/GameData/textures/lq_tech/t_wire01.png", "start": 284616185, "end": 284619121}, {"filename": "/GameData/textures/lq_tech/t_wire02.png", "start": 284619121, "end": 284622656}, {"filename": "/GameData/textures/lq_tech/t_wire03.png", "start": 284622656, "end": 284626256}, {"filename": "/GameData/textures/lq_tech/tech04_1.png", "start": 284626256, "end": 284627079}, {"filename": "/GameData/textures/lq_tech/tech04_3.png", "start": 284627079, "end": 284628548}, {"filename": "/GameData/textures/lq_tech/tech08_1.png", "start": 284628548, "end": 284639601}, {"filename": "/GameData/textures/lq_tech/tech08_2.png", "start": 284639601, "end": 284650654}, {"filename": "/GameData/textures/lq_tech/tech10_3.png", "start": 284650654, "end": 284654510}, {"filename": "/GameData/textures/lq_tech/tech14-1.png", "start": 284654510, "end": 284664781}, {"filename": "/GameData/textures/lq_tech/techbasetextures.txt", "start": 284664781, "end": 284665338}, {"filename": "/GameData/textures/lq_tech/techeye1_fbr.png", "start": 284665338, "end": 284668682}, {"filename": "/GameData/textures/lq_tech/techeye2_fbr.png", "start": 284668682, "end": 284672084}, {"filename": "/GameData/textures/lq_tech/tek_door1.png", "start": 284672084, "end": 284684230}, {"filename": "/GameData/textures/lq_tech/tek_door2.png", "start": 284684230, "end": 284696281}, {"filename": "/GameData/textures/lq_tech/tek_flr3.png", "start": 284696281, "end": 284699662}, {"filename": "/GameData/textures/lq_tech/tek_grate.png", "start": 284699662, "end": 284702540}, {"filename": "/GameData/textures/lq_tech/tek_lit1_fbr.png", "start": 284702540, "end": 284704175}, {"filename": "/GameData/textures/lq_tech/tek_lit2_fbr.png", "start": 284704175, "end": 284705161}, {"filename": "/GameData/textures/lq_tech/tek_lit3_fbr.png", "start": 284705161, "end": 284707003}, {"filename": "/GameData/textures/lq_tech/tek_lit4_fbr.png", "start": 284707003, "end": 284708130}, {"filename": "/GameData/textures/lq_tech/tek_pip1_fbr.png", "start": 284708130, "end": 284711212}, {"filename": "/GameData/textures/lq_tech/tek_pipe1.png", "start": 284711212, "end": 284714012}, {"filename": "/GameData/textures/lq_tech/tek_pipe2.png", "start": 284714012, "end": 284715671}, {"filename": "/GameData/textures/lq_tech/tek_trm1.png", "start": 284715671, "end": 284718129}, {"filename": "/GameData/textures/lq_tech/tek_trm3.png", "start": 284718129, "end": 284720739}, {"filename": "/GameData/textures/lq_tech/tek_wall4_1.png", "start": 284720739, "end": 284735184}, {"filename": "/GameData/textures/lq_tech/tele_frame1.png", "start": 284735184, "end": 284741339}, {"filename": "/GameData/textures/lq_tech/tele_frame2.png", "start": 284741339, "end": 284743290}, {"filename": "/GameData/textures/lq_tech/tele_frame3.png", "start": 284743290, "end": 284747277}, {"filename": "/GameData/textures/lq_tech/telepad1_fbr.png", "start": 284747277, "end": 284749253}, {"filename": "/GameData/textures/lq_tech/tlight11_fbr.png", "start": 284749253, "end": 284750879}, {"filename": "/GameData/textures/lq_tech/tlight12_fbr.png", "start": 284750879, "end": 284752574}, {"filename": "/GameData/textures/lq_tech/tlight13_fbr.png", "start": 284752574, "end": 284754633}, {"filename": "/GameData/textures/lq_tech/tlightblfb_fbr.png", "start": 284754633, "end": 284755206}, {"filename": "/GameData/textures/lq_tech/tlightfb_fbr.png", "start": 284755206, "end": 284755796}, {"filename": "/GameData/textures/lq_tech/tlightnb.png", "start": 284755796, "end": 284756394}, {"filename": "/GameData/textures/lq_tech/tlightrdfb_fbr.png", "start": 284756394, "end": 284756996}, {"filename": "/GameData/textures/lq_tech/treadplatemetal.png", "start": 284756996, "end": 284771750}, {"filename": "/GameData/textures/lq_tech/twall2_3.png", "start": 284771750, "end": 284775258}, {"filename": "/GameData/textures/lq_tech/w17_1.png", "start": 284775258, "end": 284792282}, {"filename": "/GameData/textures/lq_tech/w94_1.png", "start": 284792282, "end": 284805230}, {"filename": "/GameData/textures/lq_tech/z_exit_fbr.png", "start": 284805230, "end": 284806791}, {"filename": "/GameData/textures/lq_terra/afloor1_3.png", "start": 284806791, "end": 284809789}, {"filename": "/GameData/textures/lq_terra/asphalt.png", "start": 284809789, "end": 284827249}, {"filename": "/GameData/textures/lq_terra/azfloor1_1.png", "start": 284827249, "end": 284830268}, {"filename": "/GameData/textures/lq_terra/badlawn.png", "start": 284830268, "end": 284872077}, {"filename": "/GameData/textures/lq_terra/cracks1-1.png", "start": 284872077, "end": 284875096}, {"filename": "/GameData/textures/lq_terra/darkrock.png", "start": 284875096, "end": 284897665}, {"filename": "/GameData/textures/lq_terra/grass1.png", "start": 284897665, "end": 284908236}, {"filename": "/GameData/textures/lq_terra/gravel1.png", "start": 284908236, "end": 284921590}, {"filename": "/GameData/textures/lq_terra/gravel2.png", "start": 284921590, "end": 284936487}, {"filename": "/GameData/textures/lq_terra/grk_leaf1_1.png", "start": 284936487, "end": 284940480}, {"filename": "/GameData/textures/lq_terra/grk_leaf1_2.png", "start": 284940480, "end": 284944217}, {"filename": "/GameData/textures/lq_terra/marbbrn128.png", "start": 284944217, "end": 284954459}, {"filename": "/GameData/textures/lq_terra/may_drt1_1.png", "start": 284954459, "end": 284957355}, {"filename": "/GameData/textures/lq_terra/may_drt1_2.png", "start": 284957355, "end": 284960209}, {"filename": "/GameData/textures/lq_terra/may_drt2_2.png", "start": 284960209, "end": 284962953}, {"filename": "/GameData/textures/lq_terra/may_rck1_1.png", "start": 284962953, "end": 284977053}, {"filename": "/GameData/textures/lq_terra/may_rck1_2.png", "start": 284977053, "end": 284987731}, {"filename": "/GameData/textures/lq_terra/may_rck1_3.png", "start": 284987731, "end": 285000574}, {"filename": "/GameData/textures/lq_terra/may_slat1_1.png", "start": 285000574, "end": 285003422}, {"filename": "/GameData/textures/lq_terra/med_bigdirt.png", "start": 285003422, "end": 285158005}, {"filename": "/GameData/textures/lq_terra/med_bigdirt2.png", "start": 285158005, "end": 285312156}, {"filename": "/GameData/textures/lq_terra/med_bigdirt3.png", "start": 285312156, "end": 285466107}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_1.png", "start": 285466107, "end": 285480488}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_1a.png", "start": 285480488, "end": 285497462}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_2.png", "start": 285497462, "end": 285511901}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_2a.png", "start": 285511901, "end": 285528716}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_1.png", "start": 285528716, "end": 285542685}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_1a.png", "start": 285542685, "end": 285559996}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_2.png", "start": 285559996, "end": 285575070}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_2a.png", "start": 285575070, "end": 285592174}, {"filename": "/GameData/textures/lq_terra/med_cracks1.png", "start": 285592174, "end": 285605636}, {"filename": "/GameData/textures/lq_terra/med_flat1.png", "start": 285605636, "end": 285618545}, {"filename": "/GameData/textures/lq_terra/med_flat12.png", "start": 285618545, "end": 285629933}, {"filename": "/GameData/textures/lq_terra/med_flat15.png", "start": 285629933, "end": 285643034}, {"filename": "/GameData/textures/lq_terra/med_flat16.png", "start": 285643034, "end": 285655876}, {"filename": "/GameData/textures/lq_terra/med_flat2.png", "start": 285655876, "end": 285667153}, {"filename": "/GameData/textures/lq_terra/med_flat3.png", "start": 285667153, "end": 285680518}, {"filename": "/GameData/textures/lq_terra/med_flat4.png", "start": 285680518, "end": 285690467}, {"filename": "/GameData/textures/lq_terra/med_flat5.png", "start": 285690467, "end": 285701552}, {"filename": "/GameData/textures/lq_terra/med_flat5a.png", "start": 285701552, "end": 285712296}, {"filename": "/GameData/textures/lq_terra/med_flat6.png", "start": 285712296, "end": 285721281}, {"filename": "/GameData/textures/lq_terra/med_flat7.png", "start": 285721281, "end": 285732739}, {"filename": "/GameData/textures/lq_terra/med_plaster2.png", "start": 285732739, "end": 285742391}, {"filename": "/GameData/textures/lq_terra/med_rock1.png", "start": 285742391, "end": 285793648}, {"filename": "/GameData/textures/lq_terra/med_rock10.png", "start": 285793648, "end": 285802943}, {"filename": "/GameData/textures/lq_terra/med_rock10a.png", "start": 285802943, "end": 285812257}, {"filename": "/GameData/textures/lq_terra/med_rock10b.png", "start": 285812257, "end": 285825622}, {"filename": "/GameData/textures/lq_terra/med_rock10c.png", "start": 285825622, "end": 285837957}, {"filename": "/GameData/textures/lq_terra/med_rock2.png", "start": 285837957, "end": 285879834}, {"filename": "/GameData/textures/lq_terra/med_rock3.png", "start": 285879834, "end": 285925778}, {"filename": "/GameData/textures/lq_terra/med_rock3_bump.png", "start": 285925778, "end": 286041872}, {"filename": "/GameData/textures/lq_terra/med_rock4.png", "start": 286041872, "end": 286053228}, {"filename": "/GameData/textures/lq_terra/med_rock5.png", "start": 286053228, "end": 286065760}, {"filename": "/GameData/textures/lq_terra/med_rock9.png", "start": 286065760, "end": 286076784}, {"filename": "/GameData/textures/lq_terra/ret_plaster1.png", "start": 286076784, "end": 286088061}, {"filename": "/GameData/textures/lq_terra/rock1_1.png", "start": 286088061, "end": 286138432}, {"filename": "/GameData/textures/lq_terra/rock1_1b.png", "start": 286138432, "end": 286149447}, {"filename": "/GameData/textures/lq_terra/rock1_2.png", "start": 286149447, "end": 286191490}, {"filename": "/GameData/textures/lq_terra/rocks07.png", "start": 286191490, "end": 286202514}, {"filename": "/GameData/textures/lq_terra/rocks11d.png", "start": 286202514, "end": 286211828}, {"filename": "/GameData/textures/lq_terra/rocks11e.png", "start": 286211828, "end": 286221123}, {"filename": "/GameData/textures/lq_terra/sand.png", "start": 286221123, "end": 286247210}, {"filename": "/GameData/textures/lq_terra/snow1.png", "start": 286247210, "end": 286249073}, {"filename": "/GameData/textures/lq_terra/uwall1_2.png", "start": 286249073, "end": 286276724}, {"filename": "/GameData/textures/lq_terra/vines1.png", "start": 286276724, "end": 286281779}, {"filename": "/GameData/textures/lq_utility/black.png", "start": 286281779, "end": 286282327}, {"filename": "/GameData/textures/lq_utility/clip.png", "start": 286282327, "end": 286282833}, {"filename": "/GameData/textures/lq_utility/hint.png", "start": 286282833, "end": 286283699}, {"filename": "/GameData/textures/lq_utility/hintskip.png", "start": 286283699, "end": 286284592}, {"filename": "/GameData/textures/lq_utility/light_fbr.png", "start": 286284592, "end": 286285508}, {"filename": "/GameData/textures/lq_utility/origin.png", "start": 286285508, "end": 286285991}, {"filename": "/GameData/textures/lq_utility/skip.png", "start": 286285991, "end": 286286479}, {"filename": "/GameData/textures/lq_utility/star_lavaskip.png", "start": 286286479, "end": 286287592}, {"filename": "/GameData/textures/lq_utility/star_slimeskip.png", "start": 286287592, "end": 286288665}, {"filename": "/GameData/textures/lq_utility/star_waterskip.png", "start": 286288665, "end": 286290405}, {"filename": "/GameData/textures/lq_utility/trigger.png", "start": 286290405, "end": 286290902}, {"filename": "/GameData/textures/lq_wood/crate4.png", "start": 286290902, "end": 286294044}, {"filename": "/GameData/textures/lq_wood/crwdh6.png", "start": 286294044, "end": 286299052}, {"filename": "/GameData/textures/lq_wood/crwdl12.png", "start": 286299052, "end": 286301485}, {"filename": "/GameData/textures/lq_wood/crwds6.png", "start": 286301485, "end": 286302511}, {"filename": "/GameData/textures/lq_wood/may_crate3-small.png", "start": 286302511, "end": 286303502}, {"filename": "/GameData/textures/lq_wood/may_crate3.png", "start": 286303502, "end": 286306565}, {"filename": "/GameData/textures/lq_wood/may_wood1_1.png", "start": 286306565, "end": 286309405}, {"filename": "/GameData/textures/lq_wood/may_wood1_2.png", "start": 286309405, "end": 286312254}, {"filename": "/GameData/textures/lq_wood/med_ret_wood1.png", "start": 286312254, "end": 286319223}, {"filename": "/GameData/textures/lq_wood/med_wood1.png", "start": 286319223, "end": 286346591}, {"filename": "/GameData/textures/lq_wood/med_wood2.png", "start": 286346591, "end": 286354104}, {"filename": "/GameData/textures/lq_wood/med_wood2_plk1.png", "start": 286354104, "end": 286365802}, {"filename": "/GameData/textures/lq_wood/med_wood2_plk2.png", "start": 286365802, "end": 286378294}, {"filename": "/GameData/textures/lq_wood/med_wood3.png", "start": 286378294, "end": 286384294}, {"filename": "/GameData/textures/lq_wood/med_wood3_plk1.png", "start": 286384294, "end": 286397509}, {"filename": "/GameData/textures/lq_wood/med_wood4.png", "start": 286397509, "end": 286404633}, {"filename": "/GameData/textures/lq_wood/med_wood5.png", "start": 286404633, "end": 286412431}, {"filename": "/GameData/textures/lq_wood/med_wood6.png", "start": 286412431, "end": 286419211}, {"filename": "/GameData/textures/lq_wood/med_wood7.png", "start": 286419211, "end": 286428250}, {"filename": "/GameData/textures/lq_wood/med_wood8.png", "start": 286428250, "end": 286436214}, {"filename": "/GameData/textures/lq_wood/med_wood_riv1.png", "start": 286436214, "end": 286442839}, {"filename": "/GameData/textures/lq_wood/med_wood_riv1b.png", "start": 286442839, "end": 286448918}, {"filename": "/GameData/textures/lq_wood/med_wood_riv1c.png", "start": 286448918, "end": 286454042}, {"filename": "/GameData/textures/lq_wood/med_wood_riv2.png", "start": 286454042, "end": 286460452}, {"filename": "/GameData/textures/lq_wood/med_wood_riv2b.png", "start": 286460452, "end": 286466872}, {"filename": "/GameData/textures/lq_wood/med_wood_riv2c.png", "start": 286466872, "end": 286472244}, {"filename": "/GameData/textures/lq_wood/plank1.png", "start": 286472244, "end": 286478658}, {"filename": "/GameData/textures/lq_wood/plank1s.png", "start": 286478658, "end": 286480539}, {"filename": "/GameData/textures/lq_wood/plank2.png", "start": 286480539, "end": 286487119}, {"filename": "/GameData/textures/lq_wood/plank2s.png", "start": 286487119, "end": 286489008}, {"filename": "/GameData/textures/lq_wood/plank3.png", "start": 286489008, "end": 286495868}, {"filename": "/GameData/textures/lq_wood/plank3s.png", "start": 286495868, "end": 286497849}, {"filename": "/GameData/textures/lq_wood/plank4.png", "start": 286497849, "end": 286504295}, {"filename": "/GameData/textures/lq_wood/plank4s.png", "start": 286504295, "end": 286506185}, {"filename": "/GameData/textures/lq_wood/plank5.png", "start": 286506185, "end": 286511834}, {"filename": "/GameData/textures/lq_wood/sq_wood_1.png", "start": 286511834, "end": 286527875}, {"filename": "/GameData/textures/lq_wood/sq_wood_2.png", "start": 286527875, "end": 286530763}, {"filename": "/GameData/textures/lq_wood/sq_wood_2a.png", "start": 286530763, "end": 286534132}, {"filename": "/GameData/textures/lq_wood/wood_1.png", "start": 286534132, "end": 286538310}, {"filename": "/GameData/textures/lq_wood/wood_2.png", "start": 286538310, "end": 286542210}, {"filename": "/GameData/textures/lq_wood/woodbark128.png", "start": 286542210, "end": 286552417}, {"filename": "/GameData/textures/lq_wood/woodbark1m28.png", "start": 286552417, "end": 286561866}, {"filename": "/GameData/textures/lq_wood/woodbark64.png", "start": 286561866, "end": 286564803}, {"filename": "/GameData/textures/lq_wood/woodbarkA128.png", "start": 286564803, "end": 286573501}, {"filename": "/GameData/textures/lq_wood/woodbarkm64.png", "start": 286573501, "end": 286576178}, {"filename": "/GameData/textures/lq_wood/woodend.png", "start": 286576178, "end": 286578681}, {"filename": "/GameData/textures/lq_wood/woodring128.png", "start": 286578681, "end": 286587105}, {"filename": "/GameData/textures/lq_wood/woodring64.png", "start": 286587105, "end": 286589566}, {"filename": "/GameData/textures/lq_wood/woodringm128.png", "start": 286589566, "end": 286599865}, {"filename": "/GameData/textures/lq_wood/woodringm64.png", "start": 286599865, "end": 286603124}, {"filename": "/GameData/textures/metal/metal1.png", "start": 286603124, "end": 287027180}, {"filename": "/GameData/textures/muzzle_t.png", "start": 287027180, "end": 287036588}, {"filename": "/GameData/textures/muzzle_t_em.png", "start": 287036588, "end": 287045996}, {"filename": "/GameData/textures/noise/grainy5_256.png", "start": 287045996, "end": 287272038}, {"filename": "/GameData/textures/particles/blood.png", "start": 287272038, "end": 287275751}, {"filename": "/GameData/textures/particles/smoke.png", "start": 287275751, "end": 287279621}, {"filename": "/GameData/textures/particles/trail.png", "start": 287279621, "end": 287298491}, {"filename": "/GameData/textures/particles/wood.png", "start": 287298491, "end": 287313974}, {"filename": "/GameData/textures/pp/lut.png", "start": 287313974, "end": 287315597}, {"filename": "/GameData/textures/pp/main.png", "start": 287315597, "end": 287353649}, {"filename": "/GameData/textures/pp/pal.png_out.png", "start": 287353649, "end": 287359999}, {"filename": "/GameData/textures/pp/sin.png", "start": 287359999, "end": 287361324}, {"filename": "/GameData/textures/shirt.png", "start": 287361324, "end": 287553075}, {"filename": "/GameData/textures/skies/skybox1_cube.png", "start": 287553075, "end": 288766042}, {"filename": "/GameData/textures/skies/skybox2_overcast_cube.png", "start": 288766042, "end": 289413881}, {"filename": "/GameData/textures/tormentPack/+0str_bloodfall.png", "start": 289413881, "end": 289415958}, {"filename": "/GameData/textures/tormentPack/+1str_bloodfall.png", "start": 289415958, "end": 289417992}, {"filename": "/GameData/textures/tormentPack/+2str_bloodfall.png", "start": 289417992, "end": 289420090}, {"filename": "/GameData/textures/tormentPack/+3str_bloodfall.png", "start": 289420090, "end": 289422161}, {"filename": "/GameData/textures/tormentPack/+4str_bloodfall.png", "start": 289422161, "end": 289424221}, {"filename": "/GameData/textures/tormentPack/+5str_bloodfall.png", "start": 289424221, "end": 289426242}, {"filename": "/GameData/textures/tormentPack/+6str_bloodfall.png", "start": 289426242, "end": 289428288}, {"filename": "/GameData/textures/tormentPack/+7str_bloodfall.png", "start": 289428288, "end": 289430357}, {"filename": "/GameData/textures/tormentPack/str_blood.png", "start": 289430357, "end": 289432365}, {"filename": "/GameData/textures/tormentPack/str_blood_large.png", "start": 289432365, "end": 289454915}, {"filename": "/GameData/textures/tormentPack/str_bloodvein1.png", "start": 289454915, "end": 289481590}, {"filename": "/GameData/textures/tormentPack/str_bloodvein2.png", "start": 289481590, "end": 289513623}, {"filename": "/GameData/textures/tormentPack/str_bloodvein3.png", "start": 289513623, "end": 289547294}, {"filename": "/GameData/textures/tormentPack/str_bloodvein4.png", "start": 289547294, "end": 289578936}, {"filename": "/GameData/textures/tormentPack/str_bloodvein5.png", "start": 289578936, "end": 289612427}, {"filename": "/GameData/textures/tormentPack/str_bloodvein6.png", "start": 289612427, "end": 289646901}, {"filename": "/GameData/textures/tormentPack/str_bloodvein7.png", "start": 289646901, "end": 289681792}, {"filename": "/GameData/textures/tormentPack/str_bloodvein8.png", "start": 289681792, "end": 289709941}, {"filename": "/GameData/textures/tormentPack/str_bloodvein9.png", "start": 289709941, "end": 289745394}, {"filename": "/GameData/textures/tormentPack/str_metalflr1.png", "start": 289745394, "end": 289782088}, {"filename": "/GameData/textures/tormentPack/str_metalflr2.png", "start": 289782088, "end": 289818663}, {"filename": "/GameData/textures/tormentPack/str_metalflr3.png", "start": 289818663, "end": 289849656}, {"filename": "/GameData/textures/tormentPack/str_metalflr4.png", "start": 289849656, "end": 289879826}, {"filename": "/GameData/textures/tormentPack/str_metalflr5.png", "start": 289879826, "end": 289919276}, {"filename": "/GameData/textures/tormentPack/str_metalflr6.png", "start": 289919276, "end": 289958436}, {"filename": "/GameData/textures/tormentPack/str_metalflr7.png", "start": 289958436, "end": 290000128}, {"filename": "/GameData/textures/tormentPack/str_metalflr8.png", "start": 290000128, "end": 290040546}, {"filename": "/GameData/textures/tormentPack/str_metalgen1.png", "start": 290040546, "end": 290069816}, {"filename": "/GameData/textures/tormentPack/str_metalgen2.png", "start": 290069816, "end": 290099332}, {"filename": "/GameData/textures/tormentPack/str_metalgen3.png", "start": 290099332, "end": 290128808}, {"filename": "/GameData/textures/tormentPack/str_metalgen4.png", "start": 290128808, "end": 290153631}, {"filename": "/GameData/textures/tormentPack/str_metalgen5.png", "start": 290153631, "end": 290178501}, {"filename": "/GameData/textures/tormentPack/str_metalgen6.png", "start": 290178501, "end": 290202591}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl1.png", "start": 290202591, "end": 290231976}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl2.png", "start": 290231976, "end": 290264706}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl3.png", "start": 290264706, "end": 290291324}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl4.png", "start": 290291324, "end": 290320588}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl5.png", "start": 290320588, "end": 290354963}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl6.png", "start": 290354963, "end": 290393458}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl7.png", "start": 290393458, "end": 290429608}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl8.png", "start": 290429608, "end": 290467601}, {"filename": "/GameData/textures/tormentPack/str_metalpan1.png", "start": 290467601, "end": 290502461}, {"filename": "/GameData/textures/tormentPack/str_metalpan2.png", "start": 290502461, "end": 290538002}, {"filename": "/GameData/textures/tormentPack/str_metalpan3.png", "start": 290538002, "end": 290568438}, {"filename": "/GameData/textures/tormentPack/str_metalpan4.png", "start": 290568438, "end": 290599398}, {"filename": "/GameData/textures/tormentPack/str_metalpan5.png", "start": 290599398, "end": 290635770}, {"filename": "/GameData/textures/tormentPack/str_metalpan6.png", "start": 290635770, "end": 290673812}, {"filename": "/GameData/textures/tormentPack/str_metalpan7.png", "start": 290673812, "end": 290713041}, {"filename": "/GameData/textures/tormentPack/str_metalpan8.png", "start": 290713041, "end": 290752135}, {"filename": "/GameData/textures/tormentPack/str_rotwoodgen1.png", "start": 290752135, "end": 290770714}, {"filename": "/GameData/textures/tormentPack/str_rotwoodgen2.png", "start": 290770714, "end": 290790010}, {"filename": "/GameData/textures/tormentPack/str_rotwoodgen3.png", "start": 290790010, "end": 290810064}, {"filename": "/GameData/textures/tormentPack/str_stonebrk1.png", "start": 290810064, "end": 290835456}, {"filename": "/GameData/textures/tormentPack/str_stonebrk2.png", "start": 290835456, "end": 290862458}, {"filename": "/GameData/textures/tormentPack/str_stonebrk3.png", "start": 290862458, "end": 290885088}, {"filename": "/GameData/textures/tormentPack/str_stonebrk4.png", "start": 290885088, "end": 290909141}, {"filename": "/GameData/textures/tormentPack/str_stonebrk5.png", "start": 290909141, "end": 290940225}, {"filename": "/GameData/textures/tormentPack/str_stonebrk6.png", "start": 290940225, "end": 290972897}, {"filename": "/GameData/textures/tormentPack/str_stonebrk7.png", "start": 290972897, "end": 291001421}, {"filename": "/GameData/textures/tormentPack/str_stonebrk8.png", "start": 291001421, "end": 291031673}, {"filename": "/GameData/textures/tormentPack/str_stoneflr1.png", "start": 291031673, "end": 291055829}, {"filename": "/GameData/textures/tormentPack/str_stoneflr2.png", "start": 291055829, "end": 291083735}, {"filename": "/GameData/textures/tormentPack/str_stoneflr3.png", "start": 291083735, "end": 291104740}, {"filename": "/GameData/textures/tormentPack/str_stoneflr4.png", "start": 291104740, "end": 291125884}, {"filename": "/GameData/textures/tormentPack/str_stoneflr5.png", "start": 291125884, "end": 291157416}, {"filename": "/GameData/textures/tormentPack/str_stoneflr6.png", "start": 291157416, "end": 291191699}, {"filename": "/GameData/textures/tormentPack/str_stonegen1.png", "start": 291191699, "end": 291210722}, {"filename": "/GameData/textures/tormentPack/str_stonegen2.png", "start": 291210722, "end": 291230093}, {"filename": "/GameData/textures/tormentPack/str_stonegen3.png", "start": 291230093, "end": 291251865}, {"filename": "/GameData/textures/tormentPack/str_stonegen4.png", "start": 291251865, "end": 291276910}, {"filename": "/GameData/textures/tormentPack/str_stonegen5.png", "start": 291276910, "end": 291302298}, {"filename": "/GameData/textures/tormentPack/str_stonegen6.png", "start": 291302298, "end": 291329735}, {"filename": "/GameData/textures/tormentPack/str_stonerubble.png", "start": 291329735, "end": 291358129}, {"filename": "/GameData/textures/tormentPack/str_stonewall1.png", "start": 291358129, "end": 291381386}, {"filename": "/GameData/textures/tormentPack/str_stonewall2.png", "start": 291381386, "end": 291406191}, {"filename": "/GameData/textures/tormentPack/str_stonewall3.png", "start": 291406191, "end": 291430854}, {"filename": "/GameData/textures/tormentPack/str_stonewall4.png", "start": 291430854, "end": 291457324}, {"filename": "/GameData/textures/tormentPack/str_stonewall5.png", "start": 291457324, "end": 291484791}, {"filename": "/GameData/textures/tormentPack/str_stonewall6.png", "start": 291484791, "end": 291512959}, {"filename": "/GameData/textures/tormentPack/str_stonewall7.png", "start": 291512959, "end": 291542348}, {"filename": "/GameData/textures/tormentPack/str_stonewall8.png", "start": 291542348, "end": 291573244}, {"filename": "/GameData/textures/tormentPack/{str_bloodgunk.png", "start": 291573244, "end": 291599693}, {"filename": "/GameData/textures/tormentPack/{str_bloodweb1.png", "start": 291599693, "end": 291616933}, {"filename": "/GameData/textures/tormentPack/{str_bloodweb2.png", "start": 291616933, "end": 291625884}, {"filename": "/GameData/textures/tormentPack/{str_bloodweb3.png", "start": 291625884, "end": 291631895}, {"filename": "/GameData/textures/tormentPack/{str_grating1.png", "start": 291631895, "end": 291640476}, {"filename": "/GameData/textures/tormentPack/{str_grating2.png", "start": 291640476, "end": 291655679}, {"filename": "/GameData/textures/tormentPack/{str_grating3.png", "start": 291655679, "end": 291675027}, {"filename": "/GameData/textures/tormentPack/{str_grating4.png", "start": 291675027, "end": 291687184}, {"filename": "/GameData/textures/tormentPack/{str_grating5.png", "start": 291687184, "end": 291708689}, {"filename": "/GameData/textures/tormentPack/{str_grating6.png", "start": 291708689, "end": 291736935}, {"filename": "/GameData/textures/ui/circle.png", "start": 291736935, "end": 291740390}, {"filename": "/GameData/textures/ui/crosshair.png", "start": 291740390, "end": 291744505}, {"filename": "/GameData/textures/ui/crosshair_dot.png", "start": 291744505, "end": 291746179}, {"filename": "/GameData/textures/ui/crosshair_line.png", "start": 291746179, "end": 291748118}, {"filename": "/GameData/textures/ui/debuffs/disb.png", "start": 291748118, "end": 292214120}, {"filename": "/GameData/textures/ui/debuffs/qs.png", "start": 292214120, "end": 292701440}, {"filename": "/GameData/textures/ui/debuffs/stun.png", "start": 292701440, "end": 293195317}, {"filename": "/GameData/textures/ui/test_button.png", "start": 293195317, "end": 293198713}, {"filename": "/GameData/textures/ui/white.png", "start": 293198713, "end": 293198834}, {"filename": "/GameData/textures/wall/brickWall1.png", "start": 293198834, "end": 293207050}, {"filename": "/GameData/textures/wall/brickWall2.png", "start": 293207050, "end": 293215174}, {"filename": "/GameData/textures/wall/brickWall3.png", "start": 293215174, "end": 293223057}, {"filename": "/GameData/textures/water/Water1_t.png", "start": 293223057, "end": 293536158}, {"filename": "/GameData/textures/wood/wood1.png", "start": 293536158, "end": 293884510}, {"filename": "/GameData/ui/base.rcss", "start": 293884510, "end": 293885553}, {"filename": "/GameData/ui/pause.rml", "start": 293885553, "end": 293886568}, {"filename": "/GameData/ui/settings.rml", "start": 293886568, "end": 293887436}, {"filename": "/GameData/ui/style.rcss", "start": 293887436, "end": 293892435}, {"filename": "/GameData/ui/test.rml", "start": 293892435, "end": 293893561}, {"filename": "/GameData/ui/videoSettings.rml", "start": 293893561, "end": 293896046}, {"filename": "/GameData/videos/meowl.mpg", "start": 293896046, "end": 297836398}, {"filename": "/GameData/videos/writeshit.mpg", "start": 297836398, "end": 303927150}], "remote_package_size": 303927150});

  })();

// end include: C:\Users\bogda_\AppData\Local\Temp\tmp5z6hm23j.js
// include: C:\Users\bogda_\AppData\Local\Temp\tmpldc990kf.js

    // All the pre-js content up to here must remain later on, we need to run
    // it.
    if ((typeof ENVIRONMENT_IS_WASM_WORKER != 'undefined' && ENVIRONMENT_IS_WASM_WORKER) || (typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD) || (typeof ENVIRONMENT_IS_AUDIO_WORKLET != 'undefined' && ENVIRONMENT_IS_AUDIO_WORKLET)) Module['preRun'] = [];
    var necessaryPreJSTasks = Module['preRun'].slice();
  // end include: C:\Users\bogda_\AppData\Local\Temp\tmpldc990kf.js
// include: C:\Users\bogda_\AppData\Local\Temp\tmpkfrny6gx.js

    if (!Module['preRun']) throw 'Module.preRun should exist because file support used it; did a pre-js delete it?';
    necessaryPreJSTasks.forEach((task) => {
      if (Module['preRun'].indexOf(task) < 0) throw 'All preRun tasks that exist before user pre-js code should remain after; did you replace Module or modify Module.preRun?';
    });
  // end include: C:\Users\bogda_\AppData\Local\Temp\tmpkfrny6gx.js


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
  1542834: ($0) => { var str = UTF8ToString($0) + '\n\n' + 'Abort/Retry/Ignore/AlwaysIgnore? [ariA] :'; var reply = window.prompt(str, "i"); if (reply === null) { reply = "i"; } return reply.length === 1 ? reply.charCodeAt(0) : -1; },  
 1543049: () => { if (typeof(AudioContext) !== 'undefined') { return true; } else if (typeof(webkitAudioContext) !== 'undefined') { return true; } return false; },  
 1543196: () => { if ((typeof(navigator.mediaDevices) !== 'undefined') && (typeof(navigator.mediaDevices.getUserMedia) !== 'undefined')) { return true; } else if (typeof(navigator.webkitGetUserMedia) !== 'undefined') { return true; } return false; },  
 1543430: ($0) => { if(typeof(Module['SDL2']) === 'undefined') { Module['SDL2'] = {}; } var SDL2 = Module['SDL2']; if (!$0) { SDL2.audio = {}; } else { SDL2.capture = {}; } if (!SDL2.audioContext) { if (typeof(AudioContext) !== 'undefined') { SDL2.audioContext = new AudioContext(); } else if (typeof(webkitAudioContext) !== 'undefined') { SDL2.audioContext = new webkitAudioContext(); } if (SDL2.audioContext) { if ((typeof navigator.userActivation) === 'undefined') { autoResumeAudioContext(SDL2.audioContext); } } } return SDL2.audioContext === undefined ? -1 : 0; },  
 1543982: () => { var SDL2 = Module['SDL2']; return SDL2.audioContext.sampleRate; },  
 1544050: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; var have_microphone = function(stream) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); SDL2.capture.silenceTimer = undefined; SDL2.capture.silenceBuffer = undefined } SDL2.capture.mediaStreamNode = SDL2.audioContext.createMediaStreamSource(stream); SDL2.capture.scriptProcessorNode = SDL2.audioContext.createScriptProcessor($1, $0, 1); SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) { if ((SDL2 === undefined) || (SDL2.capture === undefined)) { return; } audioProcessingEvent.outputBuffer.getChannelData(0).fill(0.0); SDL2.capture.currentCaptureBuffer = audioProcessingEvent.inputBuffer; dynCall('vp', $2, [$3]); }; SDL2.capture.mediaStreamNode.connect(SDL2.capture.scriptProcessorNode); SDL2.capture.scriptProcessorNode.connect(SDL2.audioContext.destination); SDL2.capture.stream = stream; }; var no_microphone = function(error) { }; SDL2.capture.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.capture.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { SDL2.capture.currentCaptureBuffer = SDL2.capture.silenceBuffer; dynCall('vp', $2, [$3]); }; SDL2.capture.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); if ((navigator.mediaDevices !== undefined) && (navigator.mediaDevices.getUserMedia !== undefined)) { navigator.mediaDevices.getUserMedia({ audio: true, video: false }).then(have_microphone).catch(no_microphone); } else if (navigator.webkitGetUserMedia !== undefined) { navigator.webkitGetUserMedia({ audio: true, video: false }, have_microphone, no_microphone); } },  
 1545743: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; SDL2.audio.scriptProcessorNode = SDL2.audioContext['createScriptProcessor']($1, 0, $0); SDL2.audio.scriptProcessorNode['onaudioprocess'] = function (e) { if ((SDL2 === undefined) || (SDL2.audio === undefined)) { return; } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); SDL2.audio.silenceTimer = undefined; SDL2.audio.silenceBuffer = undefined; } SDL2.audio.currentOutputBuffer = e['outputBuffer']; dynCall('vp', $2, [$3]); }; SDL2.audio.scriptProcessorNode['connect'](SDL2.audioContext['destination']); if (SDL2.audioContext.state === 'suspended') { SDL2.audio.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.audio.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { if ((typeof navigator.userActivation) !== 'undefined') { if (navigator.userActivation.hasBeenActive) { SDL2.audioContext.resume(); } } SDL2.audio.currentOutputBuffer = SDL2.audio.silenceBuffer; dynCall('vp', $2, [$3]); SDL2.audio.currentOutputBuffer = undefined; }; SDL2.audio.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); } },  
 1546918: ($0, $1) => { var SDL2 = Module['SDL2']; var numChannels = SDL2.capture.currentCaptureBuffer.numberOfChannels; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.capture.currentCaptureBuffer.getChannelData(c); if (channelData.length != $1) { throw 'Web Audio capture buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } if (numChannels == 1) { for (var j = 0; j < $1; ++j) { setValue($0 + (j * 4), channelData[j], 'float'); } } else { for (var j = 0; j < $1; ++j) { setValue($0 + (((j * numChannels) + c) * 4), channelData[j], 'float'); } } } },  
 1547523: ($0, $1) => { var SDL2 = Module['SDL2']; var buf = $0 >>> 2; var numChannels = SDL2.audio.currentOutputBuffer['numberOfChannels']; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.audio.currentOutputBuffer['getChannelData'](c); if (channelData.length != $1) { throw 'Web Audio output buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } for (var j = 0; j < $1; ++j) { channelData[j] = HEAPF32[buf + (j*numChannels + c)]; } } },  
 1548012: ($0) => { var SDL2 = Module['SDL2']; if ($0) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); } if (SDL2.capture.stream !== undefined) { var tracks = SDL2.capture.stream.getAudioTracks(); for (var i = 0; i < tracks.length; i++) { SDL2.capture.stream.removeTrack(tracks[i]); } } if (SDL2.capture.scriptProcessorNode !== undefined) { SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) {}; SDL2.capture.scriptProcessorNode.disconnect(); } if (SDL2.capture.mediaStreamNode !== undefined) { SDL2.capture.mediaStreamNode.disconnect(); } SDL2.capture = undefined; } else { if (SDL2.audio.scriptProcessorNode != undefined) { SDL2.audio.scriptProcessorNode.disconnect(); } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); } SDL2.audio = undefined; } if ((SDL2.audioContext !== undefined) && (SDL2.audio === undefined) && (SDL2.capture === undefined)) { SDL2.audioContext.close(); SDL2.audioContext = undefined; } },  
 1549018: ($0, $1, $2) => { var w = $0; var h = $1; var pixels = $2; if (!Module['SDL2']) Module['SDL2'] = {}; var SDL2 = Module['SDL2']; if (SDL2.ctxCanvas !== Module['canvas']) { SDL2.ctx = Browser.createContext(Module['canvas'], false, true); SDL2.ctxCanvas = Module['canvas']; } if (SDL2.w !== w || SDL2.h !== h || SDL2.imageCtx !== SDL2.ctx) { SDL2.image = SDL2.ctx.createImageData(w, h); SDL2.w = w; SDL2.h = h; SDL2.imageCtx = SDL2.ctx; } var data = SDL2.image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = 0xff; src++; dst += 4; } } else { if (SDL2.data32Data !== data) { SDL2.data32 = new Int32Array(data.buffer); SDL2.data8 = new Uint8Array(data.buffer); SDL2.data32Data = data; } var data32 = SDL2.data32; num = data32.length; data32.set(HEAP32.subarray(src, src + num)); var data8 = SDL2.data8; var i = 3; var j = i + 4*num; if (num % 8 == 0) { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; } } else { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; } } } SDL2.ctx.putImageData(SDL2.image, 0, 0); },  
 1550484: ($0, $1, $2, $3, $4) => { var w = $0; var h = $1; var hot_x = $2; var hot_y = $3; var pixels = $4; var canvas = document.createElement("canvas"); canvas.width = w; canvas.height = h; var ctx = canvas.getContext("2d"); var image = ctx.createImageData(w, h); var data = image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = (val >> 24) & 0xff; src++; dst += 4; } } else { var data32 = new Int32Array(data.buffer); num = data32.length; data32.set(HEAP32.subarray(src, src + num)); } ctx.putImageData(image, 0, 0); var url = hot_x === 0 && hot_y === 0 ? "url(" + canvas.toDataURL() + "), auto" : "url(" + canvas.toDataURL() + ") " + hot_x + " " + hot_y + ", auto"; var urlBuf = _malloc(url.length + 1); stringToUTF8(url, urlBuf, url.length + 1); return urlBuf; },  
 1551472: ($0) => { if (Module['canvas']) { Module['canvas'].style['cursor'] = UTF8ToString($0); } },  
 1551555: () => { if (Module['canvas']) { Module['canvas'].style['cursor'] = 'none'; } },  
 1551624: () => { return window.innerWidth; },  
 1551654: () => { return window.innerHeight; },  
 1551685: () => { return Module.window ? 1 : 0; },  
 1551719: () => { var win = Module.window ? Module.window : window; var AudioContext = win.AudioContext || win.webkitAudioContext; var contextForCheck = new AudioContext(); if (!contextForCheck) { return 0; } var retValue = 0; if (self.AudioWorkletNode) { if (contextForCheck.audioWorklet.addModule) { retValue = 1; } } contextForCheck.close(); return retValue; },  
 1552067: () => { var win = Module.window ? Module.window : window; var AudioContext = win.AudioContext || win.webkitAudioContext; var infocontext = new AudioContext(); if (!infocontext) { return 0; } var inforate = infocontext.sampleRate; infocontext.close(); return inforate; },  
 1552331: () => { Module.mInputRegistered = false; },  
 1552368: () => { var win = Module.window ? Module.window : window; var AudioContext = win.AudioContext || win.webkitAudioContext; Module.context = new AudioContext(); if (!Module.context) { return 0; } Module.FMOD_JS_MixFunction = Module["cwrap"]('FMOD_JS_MixFunction', 'void', ['number']); return Module.context.sampleRate; },  
 1552680: ($0, $1) => { Module._as_script_node = Module.context.createScriptProcessor($1, 0, $0); Module["OutputWebAudio_resumeAudio"] = function() { if (Module.context && Module.mInputRegistered) { console.log('Resetting audio driver based on user input.'); Module._as_script_node.connect(Module.context.destination); Module._as_script_node.onaudioprocess = function(audioProcessingEvent) { Module._as_output_buffer = audioProcessingEvent.outputBuffer; Module.FMOD_JS_MixFunction(Module._as_output_buffer.getChannelData(0).length); }; Module.context.resume(); var win = Module.window ? Module.window : window; win.removeEventListener('click', Module.OutputWebAudio_resumeAudio, false); win.removeEventListener('touchend', Module.OutputWebAudio_resumeAudio, false); Module.mInputRegistered = false; } }; },  
 1553464: () => { if (Module.mInputRegistered) { Module.mInputRegistered = false; var win = Module.window ? Module.window : window; win.removeEventListener('click', Module["OutputWebAudio_resumeAudio"], false); win.removeEventListener('touchend', Module["OutputWebAudio_resumeAudio"], false); } },  
 1553745: () => { var win = Module.window ? Module.window : window; win.addEventListener('touchend', Module["OutputWebAudio_resumeAudio"], false); win.addEventListener('click', Module["OutputWebAudio_resumeAudio"], false); Module.mInputRegistered = true; },  
 1553986: () => { Module._as_script_node.disconnect(Module.context.destination); },  
 1554053: ($0, $1, $2, $3) => { var data = HEAPF32.subarray(($0 / 4), ($0 / 4) + ($2 * $3)); for (var channel = 0; channel < $3; channel++) { var outputData = Module._as_output_buffer.getChannelData(channel); for (var sample = 0; sample < $2; sample++) { outputData[sample+$1] = data[(sample*$3)+channel]; } } },  
 1554335: () => { Module.context.suspend(); },  
 1554365: () => { Module.context.resume(); },  
 1554394: () => { var win = Module.window ? Module.window : window; var AudioContext = win.AudioContext || win.webkitAudioContext; var infocontext = new AudioContext(); if (!infocontext) { return 0; } var inforate = infocontext.sampleRate; infocontext.close(); return inforate; },  
 1554658: () => { Module.mWorkletNode = null; Module.mModulePolling = false; Module.mModuleLoading = false; Module.mStartInterval = null; Module.mStopInterval = null; Module.mSuspendInterval = null; Module.mResumeInterval = null; Module.mWorkletNodeConnected = false; Module.mInputRegistered = false; var win = Module.window ? Module.window : window; var AudioContext = win.AudioContext || win.webkitAudioContext; Module.mContext = new AudioContext(); if (!Module.mContext) { return 0; } Module.mContext.destination.channelCount = Module.mContext.destination.maxChannelCount; return Module.mContext.destination.maxChannelCount; },  
 1555272: ($0) => { const initAddModuleRef = $0; if (!initAddModuleRef) { Module.mAddModuleRef = 0; } if (!self.AudioWorkletNode) { return -1; } if (!Module.mContext.audioWorklet.addModule) { return -2; } Module.FMOD_JS_MixerSlowpathFunction = Module["cwrap"]('FMOD_JS_MixerSlowpathFunction', 'void', []); Module.FMOD_JS_MixerFastpathFunction = Module["cwrap"]('FMOD_JS_MixerFastpathFunction', 'void', ['number']); return Module.mContext.sampleRate; },  
 1555706: ($0, $1) => { Module.mSpeakerChannelCount = $0; const bufferLength = $1; Module.mUrl = null; Module.mOutputData = null; Module.mSharedArrayBuffers = false; if (self.SharedArrayBuffer) { if (self.crossOriginIsolated) { Module.mSharedArrayBuffers = true; } } if (!Module.mSharedArrayBuffers) { const slowCodePath = new Blob( [ "class AudioProcessor extends AudioWorkletProcessor", "{", "constructor(options)", "{", "super();", "this.payload = [null, null];", "this.bufferFlag = 0;", "this.dataFlag = 0;", "this.bufferIndex = 0;", "this.bufferSize = 0;", "this.channelCount = options.outputChannelCount;", "this.port.onmessage = (event) => {", "const { data } = event;", "if (data)", "{", "this.payload[this.dataFlag] = new Float32Array(data);", "this.bufferSize = data.length / this.channelCount;", "}", "else", "{", "this.payload[this.dataFlag] = null;", "}", "this.dataFlag ^= 1;", "};", "}", "process(inputs, outputs, parameters)", "{", "const output = outputs[0];", "if (output.length === 0) return true;", "if (this.payload[this.bufferFlag]) {", "if (this.bufferIndex === 0) {", "this.port.postMessage(this.bufferSize);", "}", "const bufferSliceEnd = this.bufferSize / output[0].length;", "const sliceOffset = output[0].length * this.bufferIndex;", "for (let channel = 0; channel < output.length; ++channel) {", "const outputChannel = output[channel];", "const indexOffset = this.bufferSize * channel + sliceOffset;", "outputChannel.set(new Float32Array(this.payload[this.bufferFlag].slice(0 + indexOffset, outputChannel.length + indexOffset)));", "}", "this.bufferIndex++;", "if (this.bufferIndex === bufferSliceEnd) {", "this.bufferIndex = 0;", "this.bufferFlag ^= 1;", "}", "}", "return true;", "}", "}", "registerProcessor('audio-processor', AudioProcessor);" ], { type: 'application/javascript' }); Module.mUrl = URL.createObjectURL(slowCodePath); Module.mOutputData = new Float32Array(Module.mSpeakerChannelCount * bufferLength); } else { const fastCodePath = new Blob( [ "class AudioProcessor extends AudioWorkletProcessor", "{", "constructor(options)", "{", "super();", "this.payload = null;", "this.bufferFlag = 0;", "this.bufferIndex = 0;", "this.bufferSize = 0;", "this.bufferOffset = 0;", "this.channelCount = options.outputChannelCount;", "this.port.onmessage = (event) => {", "const { data } = event;", "if (data)", "{", "this.payload = data;", "this.bufferSize = this.payload.length / (this.channelCount * 2);", "this.bufferOffset = this.payload.length / 2;", "}", "else", "{", "this.payload = null;", "}", "};", "}", "process(inputs, outputs, parameters)", "{", "const output = outputs[0];", "if (output.length === 0) return true;", "if (this.payload) {", "if (this.bufferIndex === 0) {", "this.port.postMessage(this.bufferFlag ^ 1);", "}", "const bufferSliceEnd = this.bufferSize / output[0].length;", "const sliceOffset = output[0].length * this.bufferIndex + this.bufferOffset * this.bufferFlag;", "for (let channel = 0; channel < output.length; ++channel) {", "const outputChannel = output[channel];", "const indexOffset = this.bufferSize * channel + sliceOffset;", "outputChannel.set(new Float32Array(this.payload.slice(0 + indexOffset, outputChannel.length + indexOffset)));", "}", "this.bufferIndex++;", "if (this.bufferIndex === bufferSliceEnd) {", "this.bufferIndex = 0;", "this.bufferFlag ^= 1;", "}", "}", "return true;", "}", "}", "registerProcessor('audio-processor', AudioProcessor);" ], { type: 'application/javascript' }); Module.mUrl = URL.createObjectURL(fastCodePath); Module.mOutputData = new Float32Array(new SharedArrayBuffer(Float32Array.BYTES_PER_ELEMENT * Module.mSpeakerChannelCount * bufferLength * 2)); } Module.mModulePolling = true; Module["waitForAudioWorklet"] = function(condition, callback) { var myInterval = null; if (condition()) { myInterval = setInterval(function() { if (!condition()) { callback(); clearInterval(myInterval); } }, 50); } else { callback(); return null; } return myInterval; }; Module["OutputAudioWorklet_resumeAudio"] = function() { if (Module.mContext && Module.mInputRegistered) { console.log('Resetting audio driver based on user input.'); Module.mContext.resume(); var win = Module.window ? Module.window : window; win.removeEventListener('click', Module.OutputAudioWorklet_resumeAudio, false); win.removeEventListener('touchend', Module.OutputAudioWorklet_resumeAudio, false); Module.mInputRegistered = false; if (!Module.mModuleLoading) { Module.mModuleLoading = true; Module.mAddModuleRef++; Module.mContext.resume().then(function() { Module.mContext.audioWorklet.addModule(Module.mUrl).then(function() { if (Module.mAddModuleRef === 1) { Module.mWorkletNode = new AudioWorkletNode(Module.mContext, 'audio-processor', { 'outputChannelCount' : [Module.mSpeakerChannelCount] }); Module.mModulePolling = false; URL.revokeObjectURL(Module.mUrl); if (Module.mWorkletNode) { Module.mWorkletNode.port.postMessage(Module.mOutputData); if (Module.mSharedArrayBuffers) { Module.mWorkletNode.port.onmessage = function(event) { Module.FMOD_JS_MixerFastpathFunction(event.data); }; } else { Module.mWorkletNode.port.onmessage = function(event) { Module.FMOD_JS_MixerSlowpathFunction(); Module.mWorkletNode.port.postMessage(Module.mOutputData); }; } } else { console.log('Error when creating AudioWorkletNode: Null object'); } } Module.mAddModuleRef--; }).catch (function(err) { Module.mModulePolling = false; Module.mAddModuleRef--; console.log('Error when opening audio processor '); console.log(err) }); }).catch (function(err) { Module.mModulePolling = false; Module.mAddModuleRef--; console.log('Error with mContext.resume()'); console.log(err) }); } } }; return Module.mSharedArrayBuffers; },  
 1561361: () => { var win = Module.window ? Module.window : window; win.addEventListener('touchend', Module["OutputAudioWorklet_resumeAudio"], false); win.addEventListener('click', Module["OutputAudioWorklet_resumeAudio"], false); Module.mInputRegistered = true; },  
 1561610: () => { if (Module.mStartInterval) { clearInterval(Module.mStartInterval); } if (Module.mStopInterval) { clearInterval(Module.mStopInterval); } if (Module.mSuspendInterval) { clearInterval(Module.mSuspendInterval); } if (Module.mResumeInterval) { clearInterval(Module.mResumeInterval); } if (Module.mWorkletNode) { if (Module.mWorkletNodeConnected) { Module.mWorkletNode.disconnect(); } } if (Module.mContext) { Module.mContext.close(); } if (Module.mInputRegistered) { Module.mInputRegistered = false; var win = Module.window ? Module.window : window; win.removeEventListener('click', Module["OutputAudioWorklet_resumeAudio"], false); win.removeEventListener('touchend', Module["OutputAudioWorklet_resumeAudio"], false); } },  
 1562330: () => { Module.mStartInterval = Module["waitForAudioWorklet"](function(){ return (Module.mWorkletNode === null && Module.mModulePolling); }, function(){ if (Module.mWorkletNode) { Module.mWorkletNode.connect(Module.mContext.destination); Module.mWorkletNodeConnected = true; } }); },  
 1562607: () => { if (Module["waitForAudioWorklet"]) { Module.mStopInterval = Module["waitForAudioWorklet"](function(){ return (Module.mWorkletNode === null && Module.mModulePolling || !Module.mWorkletNodeConnected); }, function(){ if (Module.mWorkletNode) { Module.mWorkletNode.disconnect(); Module.mWorkletNodeConnected = false; } }); } },  
 1562932: ($0, $1, $2) => { const buffer = $0; const bufferLength = $1; const speakerModeChannels = $2; var data = HEAPF32.subarray((buffer / 4), (buffer / 4) + (bufferLength * speakerModeChannels)); for (var channel = 0; channel < speakerModeChannels; channel++) { const offset = channel * bufferLength; for (var sample = 0; sample < bufferLength; sample++) { Module.mOutputData[sample + offset] = data[(sample * speakerModeChannels) + channel]; } } },  
 1563359: ($0, $1, $2, $3) => { const buffer = $0; const bufferLength = $1; const speakerModeChannels = $2; const frameFlag = $3; var data = HEAPF32.subarray((buffer / 4), (buffer / 4) + (bufferLength * speakerModeChannels)); const arrayOffset = speakerModeChannels * frameFlag * bufferLength; for (var channel = 0; channel < speakerModeChannels; channel++) { const offset = channel * bufferLength + arrayOffset; for (var sample = 0; sample < bufferLength; sample++) { Module.mOutputData[sample + offset] = data[(sample * speakerModeChannels) + channel]; } } },  
 1563890: () => { if (Module.mContext) { Module.mContext.suspend(); } },  
 1563946: () => { if (Module.mContext) { Module.mContext.resume(); } },  
 1564001: () => { debugger; }
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

