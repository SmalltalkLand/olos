var path = require('path');

module.exports = {
  entry: './js/index.js',
  output: {
    path: path.resolve(__dirname, 'dist'),
    filename: '[name].js'
  },
  loaders: [
    {
      test: /\.wasm$/,
      loaders: ['wasm-loader']
    }
  ]
};