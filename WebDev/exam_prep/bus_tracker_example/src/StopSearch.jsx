import { createContext, useContext, useEffect, useState } from "react"
import "./StopSearch.css"
import routesContext from "./routesContext"
import StopListing from "./StopListing"

const pigottLocation = {
    lat: 47.610671,
    lon: -122.318243
}
  
async function loadStopData(location, abortSignal) {
    const result = await fetch(
        `/api/where/stops-for-location.json?lat=${location.lat}&lon=${location.lon}&radius=200`,
        {
            signal: abortSignal
        }
    )
    if(result.ok) {
        const { data } = await result.json()
        const stops = data.list
        const routes = data.references.routes
        // note: producing a new object on each iteration is totally unnecessary here, but
        // I wanted to emulate a Redux reducer to point out the similarities
        const routesById = routes.reduce((acc, route) => ({ ...acc, [route.id]: route }), {})
        return {
            stops,
            routesById
        }
    } else {
        const responseText = await result.text()
        throw new Error(`Received status code ${result.status}: ${responseText}`)
    }
}

export default function StopSearch() {
    const [location, setLocation] = useState(pigottLocation)
    const [isLoading, setIsLoading] = useState(true)
    const [error, setError] = useState()
    const [stops, setStops] = useState()
    const [routesById, setRoutesById] = useState()

    useEffect(() => {
        const abortController = new AbortController()

        const loadData = async () => {
            try {
                const { stops, routesById } = await loadStopData(location, abortController.signal)
                setStops(stops)
                setRoutesById(routesById)
                setError()
                setIsLoading(false)
            } catch(e) {
                if(e.name !== 'AbortError') {
                    setError(e.text)
                    setIsLoading(false)
                }
            }
        }
        loadData()

        return () => {
            abortController.abort()
        }
    }, [location])

    if(isLoading) {
        return <div>Loading&hellip;</div>
    }

    if(error) {
        return <div id="error">{error}</div>
    }

    return (
        <section className="stop-search-results-list">
            <routesContext.Provider value={routesById}>
                <ul>
                    {
                        stops.map(stop => (
                            <li key={stop.id}>
                                <StopListing stop={stop} />
                            </li>
                        ))
                    }
                </ul>
            </routesContext.Provider>
        </section>
    )
}
