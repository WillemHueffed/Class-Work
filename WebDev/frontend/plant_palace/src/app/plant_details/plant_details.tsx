import Link from 'next/link';
import { useRouter } from "next/router";

function PlantDetailsComponent({ params }: {params: {name: string, species: string, cultivar: string, stage: string, jpg: string }}) {
  const router = useRouter();
  const { name, species, cultivar, stage } = router.query;
  return (
    <div>
      <h1>Plant Details</h1>
      <p>Name: {name}</p>
      <p>Species: {species}</p>
      <p>Cultivar: {cultivar}</p>
      <p>Stage: {stage}</p>
    </div>
  );
}

export default PlantDetailsComponent;
