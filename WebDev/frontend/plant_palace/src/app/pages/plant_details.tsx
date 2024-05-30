import Link from 'next/link';

function PlantDetailsComponent() {
  return (
    <div>
      {/* Use Link component to navigate to the about page */}
      <Link href="/about">
        <a>About Page</a>
      </Link>
    </div>
  );
}

export default PlantDetailsComponent;
