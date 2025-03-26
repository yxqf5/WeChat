const path = require('path');
const grpc = require('@grpc/grpc-js');
const protoLoader = require('@grpc/proto-loader');


const PROTO_PATH = path.join(__dirname,'message.proto');
const packageDefinition = protoLoader.loadSync(PROTO_PATH, {keepCase:true, longs:String,
    enums:String, defaults:true, oneofs:true})//同步的方式加载和解析




   const protoDescriptor =  grpc.loadPackageDefinition(packageDefinition);

   const message_proto = protoDescriptor.message

   module.exports = message_proto