ACCESS_TOKEN=$(curl -v --request POST \
  --url 'localhost:3001/oauth/token' \
  --header 'content-type: application/json' \
  --data '{"client_id":"TtqkDALr7skNQZykdGPp2e4UO03E7LNt","client_secret":"X2ikQTGCxgGZf5_6XozZFL-iSi-DAQtAY48PO4xRsnZywboYZY9uk72bYrHUqois","audience":"mongodb+srv://whueffed:whueffed@webdev.nque75t.mongodb.net/?retryWrites=true&w=majority&appName=WebDev,"grant_type":"password","username":"123","password":"123!1aAaaa"}' \
  | jq -r '.access_token')

echo $ACCESS_TOKEN
