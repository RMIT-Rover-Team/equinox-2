import './styles/global.scss';

import { useState } from 'react';

import Layout from './components/layout/Layout';
import type { Page } from './components/navbar/Navbar';
import Drive from './views/drive/Drive';
import Arm from './views/arm/Arm';
import Science from './views/science/Science';
import Excavator from './views/excavator/Excavator';

function App() {
	const [page, setPage] = useState<Page>('Drive');

	return (
		<Layout
			page={page}
			onPageChange={setPage}
		>
			{page === 'Drive' && <Drive />}
			{page === 'Arm' && <Arm />}
			{page === 'Science' && <Science />}
			{page === 'Excavator' && <Excavator />}
		</Layout>
	);
}

export default App;
