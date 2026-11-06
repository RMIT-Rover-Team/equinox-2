import type { ReactNode } from 'react';

import Cameras from '../cameras/Cameras';
import Navbar from '../navbar/Navbar';
import type { Page } from '../navbar/Navbar';
import styles from './Layout.module.scss';

type LayoutProps = {
	page: Page;
	onPageChange: (page: Page) => void;
	children: ReactNode;
};

function Layout({ page, onPageChange, children }: LayoutProps) {
	return (
		<div className={styles.layout}>
			<Navbar
				page={page}
				onPageChange={onPageChange}
			/>
			<Cameras />
			<main className={styles.content}>{children}</main>
		</div>
	);
}

export default Layout;
