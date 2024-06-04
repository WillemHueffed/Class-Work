
// ids to meanings: https://developers.google.com/transit/gtfs/reference?csw=1#routestxt
const routeTypeIcons = {
    0: '🚋',
    1: '🚇',
    2: '🚂',
    3: '🚌',
    4: '🚢',
    5: '🚋',
    6: '🚠',
    7: '🚞',
    11: '🚎',
    12: '🚝'
}

const getRouteTypeIcon = type => routeTypeIcons[type] || '🚌'

export default getRouteTypeIcon