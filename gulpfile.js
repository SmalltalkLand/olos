const { parallel, series } = require('gulp');
const { exec } = require('child_process');
const webpack = require('webpack-stream');
var cpp, wp, emcc;
gulp.task('cpp',cpp = exec.bind(null,'g++ ./cpp/OLOS/main.cpp -o ./cpp.o -c'));
gulp.task('emcc',emcc = exec.bind(null,'emcc -o ./theWasm.wasm ./cpp/OLOS/main.cpp'))
gulp.task('wp',wp = function(){return webpack(require('./webpack.config.js'))});
gulp.task('all',series(parallel(cpp),parallel(series(emcc,wp))));