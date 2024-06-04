import { useContext, useState } from 'react'
import routesContext from "./routesContext"
import getRouteTypeIcon from './routeTypeIcons'

function StopRouteListing({ routeId }) {
    const routesById = useContext(routesContext)
    const route = routesById[routeId]

    return (
        <section>
            <h3><a href="#">{route.shortName}: {route.description}</a></h3>            
            <span>{getRouteTypeIcon(route.type)}</span>
            <a target="_blank" href={route.url}>Agency page</a>
        </section>
    )
}

function ArrivalDetails({ arrival }) {
    const now = new Date().valueOf()
    const timeToArrival = ((arrival.predictedArrivalTime || arrival.scheduledArrivalTime) - now) / 1000 / 60
    return (
        <article>
            <h3>{arrival.routeShortName} to {arrival.tripHeadsign}</h3>
            <div>Arrives in {timeToArrival.toFixed(0)} minutes</div>
        </article>
    )
}

function ArrivalsListing({ stopId, stopName }) {
    const [arrivals, setArrivals] = useState([])

    const loadArrivals = async () => {
        const result = await fetch(`/api/where/arrivals-and-departures-for-stop/${stopId}.json`)
        const { data } = await result.json()
        console.log(data)
        setArrivals(data.entry.arrivalsAndDepartures)
    }

    return (
        <div>
            <div class="stop-details-top-row">
                <h2><a href="#">{stopName}</a></h2>
                <button className="load-arrivals-button" onClick={loadArrivals}>Load arrivals</button>
            </div>
            <ul>
                {
                    arrivals.map(arrival => (
                        <li key={arrival.tripId}>
                            <ArrivalDetails arrival={arrival} />
                        </li>
                    ))
                }
            </ul>
        </div>
    )
}

export default function StopDetails({ stop }) {
    return (
        <>
            <ArrivalsListing stopId={stop.id} stopName={stop.name} />
            <ul>
                {
                    stop.routeIds.map(routeId => (
                        <li key={routeId}>
                            <StopRouteListing routeId={routeId} />
                        </li>
                    ))
                }
            </ul>
        </>
    )
}