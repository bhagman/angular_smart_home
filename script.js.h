const char SCRIPT_JS[] PROGMEM = R"=====(
'use strict';var app=angular.module('myApp',['ngMaterial']);myDash.$inject=['$scope','$mdToast','$http','$interval','$sce','$timeout'],angular.module('myApp').controller('dash',myDash).config(['$mdThemingProvider',function(a){a.theme('custom').primaryPalette('blue-grey').accentPalette('deep-orange')}]);function myDash(a,b,e,f,g,h){a.dash={},a.dash.time_of_day='--',a.dash.next_event_due='--',a.dash.app_name='--',a.dash.app_version='--',a.dash.is_dst=!1,a.dash.is_skipping_next=!1,a.dash.is_powered=!1,a.dash.mode='--',a.dash.is_using_timer=!1,a.dash.percentage=0,a.dash.request=null,a.dash.perc_label='--',a.dash.last_action='--',a.dash.events=[],a.doAction=function(j){console.log(a.dash);var k=a.dash.request.base_url+'?',l=!1;switch(j){case'percentage':a.dash.is_powered=0<a.dash.percentage,l=a.dash.request.start_param+'='+a.dash.percentage;break;case'toggle':if(a.dash.is_powered=!a.dash.is_powered,'percentage'==a.dash.mode){a.dash.is_powered?0==a.dash.percentage&&(a.dash.percentage=1):a.dash.percentage=0,l=a.dash.request.start_param+'='+a.dash.percentage;break}else;case'master':l=a.dash.request.master_param+'='+('momentary'==a.dash.mode||(a.dash.is_powered?'true':'false'));break;case'timer':l='timer='+(a.dash.is_using_timer?'true':'false');break;case'skip':l='skip='+(a.dash.is_skipping_next?'true':'false');break;default:alert('did not understand');}e.get(k+l).then(function(m){a.showToast(m.data.message)})},a.showToast=function(j){b.show(b.simple().textContent(j).position('top right'))},a.loc_getStatus=function(){e.get('features.json').then(function(j){console.log('received',j.data),a.dash=j.data,document.title=j.data.app_name,a.showToast('Synchronised')})},a.loc_refreshDevices=function(){console.log('do it'),a.loc.ips_to_check=[],a.loc.ips_counted=0,a.loc.ips_checked=0,a.loc.ip_scan_percentage=0;for(var j=0;j<a.loc.devices.length;j++)a.loc.ips_to_check.push({ip_address:'http://'+a.loc.devices[j].address,checked:!1,result:'Held in queue'});a.loc_try_next_in_list()},a.getPowerStyle=function(){if(!a.dash.is_powered)return{color:'rgba(255, 255, 255, 0.3)'}},a.loc_getStatus(),f(function(){a.loc_getStatus()},60000),a.loc_detect_devices=function(){localStorage.setItem('loc.scan',JSON.stringify(a.loc.scan)),a.loc.ips_to_check=[],a.loc.ips_counted=0,a.loc.replace_detected&&(a.loc.devices=[]);for(var k=a.loc.scan.ip_range_start;k<=a.loc.scan.ip_range_end;k++)a.loc.ips_to_check.push({ip_address:'http://192.168.0.'+k,checked:!1,result:'Held in queue'});a.loc.ips_checked=0,a.loc.ip_scan_percentage=0,h(function(){a.loc_try_next_in_list()},2000),a.loc_doStore()},a.loc_device_merged_in=function(j){console.log('checking',j);for(var k=j.address,l=0;l<a.loc.devices.length;l++)if(a.loc.devices[l].address==j.address)return a.loc.devices[l]=j,console.log('merged'),!0;return!1},a.loc_try_next_in_list=function(){for(var j=0;j<a.loc.ips_to_check.length;j++)if(a.loc.ips_to_check[j].checked);else{console.log('kicking off a test for ',a.loc.ips_to_check[j]);var k=a.loc.ips_to_check[j].ip_address+'/features.json',l=g.trustAsResourceUrl(k);let m=j;return a.loc.ips_to_check[m].result='waiting for response...',e.jsonp(l,{jsonpCallbackParam:'callback',timeout:1e4}).then(function(o){console.log('got something!',o.data),a.loc.ips_to_check[m].result='Found device '+o.data.app_name,a.loc_device_merged_in(o.data)?a.loc.ips_to_check[m].result='Updated device '+o.data.app_name:(a.loc.devices.push(o.data),console.log('added it')),a.loc.store_detected&&a.loc_doStore()}).catch(function(){console.log('checked off:',m),a.loc.ips_to_check[m].result='No response'}),a.loc.ips_to_check[j].checked=!0,a.loc.ips_checked++,a.loc.ip_scan_percentage=parseInt(100*(a.loc.ips_checked/a.loc.ips_to_check.length)),void h(function(){a.loc_try_next_in_list()},2000)}console.log('all sites done.'),a.loc.ips_to_check=[]},a.remoteRequest=function(j){window.loc_devices=a.loc.devices;var k='http://'+j;console.log('requesting: '+k);var l=g.trustAsResourceUrl(k);e.jsonp(l,{jsonpCallbackParam:'callback'}).then(function(n){console.log(n.data),a.showToast(n.data.message)}).catch(function(){})},a.getUUID=function(){var j=new Date().getTime();window.performance&&'function'==typeof window.performance.now&&(j+=performance.now());var k='xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g,function(l){var m=0|(j+16*Math.random())%16;return j=Math.floor(j/16),('x'==l?m:8|3&m).toString(16)});return k},a.loc_doStore=function(){localStorage.setItem('loc.devices',JSON.stringify(a.loc.devices))},a.tpl_authenticate=function(){a.tpl.UUID=a.getUUID();var j={method:'login',params:{appType:'Kasa_Android',cloudUserName:a.tpl.creds.username,cloudPassword:a.tpl.creds.password,terminalUUID:a.tpl.UUID}};e.post('https://wap.tplinkcloud.com/',j).then(function(l){l.data.error_code?alert(l.data.msg):(a.tpl.token.value=l.data.result.token,localStorage.setItem('tpl_uuid',a.tpl.UUID),a.tpl.creds.store?localStorage.setItem('tpl.creds',JSON.stringify(a.tpl.creds)):localStorage.removeItem('tpl.creds'),a.tpl.token.store?localStorage.setItem('tpl.token',JSON.stringify(a.tpl.token)):localStorage.removeItem('tpl.token'),a.tpl_refreshDevices())},function(l){a.myWelcome=l.statusText})},a.tpl_refreshDevices=function(){e.post('https://wap.tplinkcloud.com?token='+a.tpl.token.value,{method:'getDeviceList'}).then(function(l){if(a.tpl.devices=l.data.result.deviceList,console.log(a.tpl.devices),a.tpl.devices.length){for(var m=0;m<a.tpl.devices.length;m++)a.tpl_getState(m);a.selected_tab_index=0}})},a.tpl_getState=function(j){var k=a.tpl.devices[j].appServerUrl,l=a.tpl.devices[j].deviceId;e.post(k+'?token='+a.tpl.token.value,{method:'passthrough',params:{deviceId:l,requestData:'{"system":{"get_sysinfo":null},"emeter":{"get_realtime":null}}'}}).then(function(o){window.response=o;var p=JSON.parse(o.data.result.responseData).system.get_sysinfo.relay_state;console.log(j,p,o),a.tpl.devices[j].is_powered=!0==p})},a.tpl_setState=function(j,k){var l=a.tpl.devices[j].appServerUrl,m=a.tpl.devices[j].deviceId,n={method:'passthrough',params:{deviceId:m,requestData:'{"system":{"set_relay_state":{"state":'+(k?1:0)+'}}}'}};e.post(l+'?token='+a.tpl.token.value,n).then(function(p){window.response=p,console.log(p)})},a.tpl={},a.tpl.refresh_rate=60,a.tpl.devices=[],a.tpl.creds=JSON.parse(localStorage.getItem('tpl.creds')),a.tpl.token=JSON.parse(localStorage.getItem('tpl.token')),null===a.tpl.creds&&(a.tpl.creds={},a.tpl.creds.username='',a.tpl.creds.password='',a.tpl.creds.store=!1),a.tpl.UUID=localStorage.getItem('tpl_uuid'),null===a.tpl.token?(a.tpl.token={},a.tpl.token.value='',a.tpl.token.store=!0):a.tpl_refreshDevices(),a.loc={},a.loc.store_detected=!0,a.loc.devices=JSON.parse(localStorage.getItem('loc.devices')),null==a.loc.devices&&(a.loc.devices=[]),a.loc.scan=JSON.parse(localStorage.getItem('loc.scan')),null===a.loc.scan&&(a.loc.scan={},a.loc.scan.ip_range_start=2,a.loc.scan.ip_range_end=40),a.selected_tab_index=0,f(function(){for(var j=0;j<a.tpl.devices.length;j++)a.tpl_getState(j)},1e3*a.tpl.refresh_rate)})=====";
