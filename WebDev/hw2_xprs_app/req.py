import requests

# Define the URL
url = "https://dev-7qw8fvj8e40n61ha.us.auth0.com/authorize"

# Define the query parameters
params = {
    "response_type": "code",
    "client_id": "JUEyADUc6kHJauEJzGAVqMLsdDVKg5uF",
    "redirect_uri": "http://localhost:3001/callback",
    "scope": "{scope}",
    "audience": "{apiAudience}",
    "state": "{state}",
}

# Make the GET request
response = requests.get(url, params=params)

# Print the response status code and content
print("Response status code:", response.status_code)
print("Response content:", response.text)
