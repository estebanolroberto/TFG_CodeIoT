var createError = require('http-errors');
var express = require('express');
var path = require('path');
var cookieParser = require('cookie-parser');
var logger = require('morgan');
var mongoose = require('mongoose');
var debug = require('debug')('appSmartCity:server');
var bodyParser = require("body-parser");
var cors = require('cors');

const dotenv = require('dotenv');
// get config vars
dotenv.config();

var indexRouter = require('./routes/index');
var usersRouter = require('./routes/users');
var sensorTempRouter = require('./routes/sensorHTU');
var sensorBMP280 = require('./routes/sensorBMP');
var app = express();

// view engine setup
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'pug');

app.use(logger('dev'));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));

app.use(cors());
app.use(bodyParser.json({limit: '50mb'}));
app.use(bodyParser.urlencoded({limit: '50mb', extended: true}));

app.use('/', indexRouter);
app.use('/users', usersRouter);
app.use('/sensorHTU', sensorTempRouter);
app.use('/sensorBMP', sensorBMP280);

// catch 404 and forward to error handler
app.use(function(req, res, next) {
  next(createError(404));
});

// error handler
app.use(function(err, req, res, next) {
  // set locals, only providing error in development
  res.locals.message = err.message;
  res.locals.error = req.app.get('env') === 'development' ? err : {};

  // render the error page
  res.status(err.status || 500);
  res.render('error');
});

 // MongoDB Atlas DB cluster connection
 mongoose.set("strictQuery", false);
 mongoose
 .connect(
   process.env.MONGODB,
   { useNewUrlParser: true, useUnifiedTopology: true }
 )
 .then(() => debug("MongoDB Atlas DataBase connection successful"));
 
module.exports = app;
