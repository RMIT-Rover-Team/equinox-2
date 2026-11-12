import type { ReactNode } from 'react';
import Banner from '../banner/Banner';
import type { InfoBox } from '../banner/Banner';
import Navbar from '../navbar/Navbar';
import type { Page } from '../../types/Pages';
import styles from './Layout.module.scss';

type LayoutProps = {
	page: Page;
	onPageChange: (page: Page) => void;
	infoBoxes: InfoBox[];
	children: ReactNode;
};

function Layout({ page, onPageChange, infoBoxes, children }: LayoutProps) {
	return (
		<div className={styles.layout}>
			<Banner pageName={page} infoBoxes={infoBoxes}/>
			<Navbar page={page} onPageChange={onPageChange}/>
			<main className={styles.content}>{children}</main>
		</div>
	);
}

export default Layout;
