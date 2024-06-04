import { useState } from "react";

const sampleRouteInfo = {
    "agencyId": "23",
    "color": "",
    "description": "Streetcar: Pioneer Square - Capitol Hill",
    "id": "23_102638",
    "longName": "",
    "shortName": "First Hill Streetcar",
    "textColor": "",
    "type": 0,
    "url": "https://www.seattle.gov/transportation/getting-around/transit/streetcar/first-hill-line"
}

const sampleStopsData = [
    {
        "code": "11175",
        "direction": "",
        "id": "1_11175",
        "lat": 47.618156,
        "locationType": 0,
        "lon": -122.320747,
        "name": "Broadway And Denny",
        "parent": "",
        "routeIds": [
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "1551",
        "direction": "",
        "id": "1_1551",
        "lat": 47.599274,
        "locationType": 0,
        "lon": -122.333282,
        "name": "Occidental Mall",
        "parent": "",
        "routeIds": [
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "1651",
        "direction": "W",
        "id": "1_1651",
        "lat": 47.599262,
        "locationType": 0,
        "lon": -122.327263,
        "name": "5th & Jackson",
        "parent": "",
        "routeIds": [
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "1652",
        "direction": "E",
        "id": "1_1652",
        "lat": 47.599125,
        "locationType": 0,
        "lon": -122.327286,
        "name": "5th & Jackson",
        "parent": "",
        "routeIds": [
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "1661",
        "direction": "W",
        "id": "1_1661",
        "lat": 47.599266,
        "locationType": 0,
        "lon": -122.323341,
        "name": "7th & Jackson",
        "parent": "",
        "routeIds": [
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "1662",
        "direction": "E",
        "id": "1_1662",
        "lat": 47.599125,
        "locationType": 0,
        "lon": -122.323395,
        "name": "7th & Jackson",
        "parent": "",
        "routeIds": [
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "1671",
        "direction": "W",
        "id": "1_1671",
        "lat": 47.599274,
        "locationType": 0,
        "lon": -122.315536,
        "name": "12th & Jackson",
        "parent": "",
        "routeIds": [
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "1672",
        "direction": "E",
        "id": "1_1672",
        "lat": 47.59914,
        "locationType": 0,
        "lon": -122.315529,
        "name": "12th & Jackson",
        "parent": "",
        "routeIds": [
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "1681",
        "direction": "S",
        "id": "1_1681",
        "lat": 47.600575,
        "locationType": 0,
        "lon": -122.31424,
        "name": "14th & Washington",
        "parent": "",
        "routeIds": [
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "1682",
        "direction": "N",
        "id": "1_1682",
        "lat": 47.600594,
        "locationType": 0,
        "lon": -122.314041,
        "name": "14th & Washington",
        "parent": "",
        "routeIds": [
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "27420",
        "direction": "W",
        "id": "1_27420",
        "lat": 47.601757,
        "locationType": 0,
        "lon": -122.320297,
        "name": "Yesler & Broadway",
        "parent": "",
        "routeIds": [
            "1_100161",
            "1_100249",
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "27500",
        "direction": "E",
        "id": "1_27500",
        "lat": 47.601624,
        "locationType": 0,
        "lon": -122.319832,
        "name": "Yesler & Broadway",
        "parent": "",
        "routeIds": [
            "1_100161",
            "1_100249",
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "41904",
        "direction": "N",
        "id": "1_41904",
        "lat": 47.60545,
        "locationType": 0,
        "lon": -122.320648,
        "name": "Broadway & Terrace St",
        "parent": "",
        "routeIds": [
            "1_100289",
            "1_100223",
            "1_100447",
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "41908",
        "direction": "N",
        "id": "1_41908",
        "lat": 47.610157,
        "locationType": 0,
        "lon": -122.320663,
        "name": "Broadway & Marion",
        "parent": "",
        "routeIds": [
            "1_100289",
            "1_100223",
            "1_100447",
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "41970",
        "direction": "N",
        "id": "1_41970",
        "lat": 47.615166,
        "locationType": 0,
        "lon": -122.320702,
        "name": "Broadway & Pike-Pine",
        "parent": "",
        "routeIds": [
            "1_100289",
            "1_100223",
            "1_100447",
            "1_100249",
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "41980",
        "direction": "S",
        "id": "1_41980",
        "lat": 47.61414,
        "locationType": 0,
        "lon": -122.320885,
        "name": "Broadway & Pike-Pine",
        "parent": "",
        "routeIds": [
            "1_100289",
            "1_100223",
            "1_100249",
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "41986",
        "direction": "S",
        "id": "1_41986",
        "lat": 47.609745,
        "locationType": 0,
        "lon": -122.320869,
        "name": "Broadway & Marion",
        "parent": "",
        "routeIds": [
            "1_100289",
            "1_100223",
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    },
    {
        "code": "41988",
        "direction": "S",
        "id": "1_41988",
        "lat": 47.604641,
        "locationType": 0,
        "lon": -122.320854,
        "name": "Broadway & Terrace St",
        "parent": "",
        "routeIds": [
            "23_102638"
        ],
        "wheelchairBoarding": "UNKNOWN"
    }
]

export default function RouteDetails({ routeId }) {
    const [routeInfo, setRouteInfo] = useState(sampleRouteInfo)
    const [stops, setStops] = useState(sampleStopsData)

    return (
        <main>
            <h1>Route {routeInfo.shortName}</h1>
            <p>{routeInfo.description}</p>
            <ul>
                {
                    stops.map(stop => (
                        <li key={stop.id}>
                            <a href="#">{stop.name}</a>
                        </li>
                    ))
                }
            </ul>
        </main>
    )
}