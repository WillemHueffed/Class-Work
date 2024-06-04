import StopDetails from './StopDetails'


export default function StopListing({ stop }) {
    return (
        <article className="stop-listing">
            <StopDetails stop={stop} />
        </article>
    )
}