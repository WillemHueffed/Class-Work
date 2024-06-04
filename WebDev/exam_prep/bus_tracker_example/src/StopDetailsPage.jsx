import { useEffect, useState } from 'react'
import StopDetails from './StopDetails'


export default function StopDetailsPage({ stopId }) {
    const [stop, setStop] = useState()

    useEffect(() => {
        (async () => {
            const result = await fetch(`/api/where/stop/${stopId}.json`)
            const { data } = await result.json()
            setStop(data.entry)
        })()
    }, [])

    return (
        <article className="stop-listing">
            <h2><a href="#">{stop.name}</a></h2>
            <StopDetails stop={stop} />
        </article>
    )
}