var express = require('express');
var app = express();
var bodyParser = require('body-parser');
var request = require('request');
var crypto = require("crypto");
var async = require('async');

app.set('port', (process.env.PORT || 8000));

app.use(bodyParser.urlencoded({
	extended: true
}));

app.use(bodyParser.json());

function validate_signature(signature, body) {
	return signature == crypto.createHmac('sha256', process.env.LINE_CHANNEL_SECRET).update(new Buffer(JSON.stringify(body), 'utf8')).digest('base64');
}


app.post('/callback', function(req, res) {
	async.waterfall([
			function(callback) {
				if (!validate_signature(req.headers['x-line-signature'], req.body)) {
					return;
				}
				console.log("This line ");
				console.log("DIAG type=" + req.body['events'][0]['type']);

				if (req.body['events'][0]['source']['type'] == 'user') {
					var user_id = req.body['events'][0]['source']['userId'];
					var get_profile_options = {
						url: 'https://api.line.me/v2/bot/profile/' + user_id,
						proxy: process.env.FIXIE_URL,
						json: true,
						headers: {
							'Authorization': 'Bearer {' + process.env.LINE_CHANNEL_ACCESS_TOKEN + '}'
						}
					};
					request.get(get_profile_options, function(error, response, body) {
						if (!error && response.statusCode == 200) {
							callback(body['displayName']);
						}
					});
				}
			},
		],
		function(displayName) {
			var headers = {
				'Content-Type': 'application/json',
				'Authorization': 'Bearer {' + process.env.LINE_CHANNEL_ACCESS_TOKEN + '}',
			};

			console.log("WHO : ", displayName);
			var data = {
				'replyToken': req.body['events'][0]['replyToken'],
				"messages": [{
					"type": "text",
					"text": displayName + 'が反応していますぜ'
				}]
			};

			//オプションを定義
			var options = {
				url: 'https://api.line.me/v2/bot/message/reply',
				proxy: process.env.FIXIE_URL,
				headers: headers,
				json: true,
				body: data
			};

			request.post(options, function(error, response, body) {
				if (!error && response.statusCode == 200) {
					console.log('Status 200 OKOK : ' + body);
				} else {
					console.log('error: ' + JSON.stringify(response));
				}
			});
		}
	);
});

app.listen(app.get('port'), function() {
	console.log('Node app is running');
});

	