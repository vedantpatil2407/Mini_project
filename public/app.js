/**
 * Apex Drive - Production-Grade C++ Car Rental Client
 * Communicates natively with the C++ Crow REST API
 */

document.addEventListener('DOMContentLoaded', () => {
    // Application State
    let fleet = [];
    let rentals = [];
    let activeFilter = 'all';
    let searchQuery = '';
    let selectedCarForBooking = null;

    // DOM Elements - Fleet & Stats
    const fleetContainer = document.getElementById('fleet-container');
    const fleetSearchInput = document.getElementById('fleet-search');
    const filterButtons = document.querySelectorAll('.filter-pill');
    const statTotal = document.getElementById('stat-total');
    const statAvailable = document.getElementById('stat-available');
    const statRented = document.getElementById('stat-rented');
    const statRevenue = document.getElementById('stat-revenue');

    // DOM Elements - Active Rentals & Ledger
    const rentalsTbody = document.getElementById('rentals-tbody');
    const historyTbody = document.getElementById('history-tbody');
    const refreshRentalsBtn = document.getElementById('refresh-rentals-btn');

    // DOM Elements - Booking Modal
    const bookingModal = document.getElementById('booking-modal');
    const bookForm = document.getElementById('book-form');
    const bookCarIdInput = document.getElementById('book-car-id');
    const bookCustomerInput = document.getElementById('book-customer');
    const bookDaysInput = document.getElementById('book-days');
    const modalCarBadge = document.getElementById('modal-car-badge');
    const modalCarName = document.getElementById('modal-car-name');
    const modalCarRate = document.getElementById('modal-car-rate');
    const calcDays = document.getElementById('calc-days');
    const calcRate = document.getElementById('calc-rate');
    const calcBase = document.getElementById('calc-base');
    const calcTotal = document.getElementById('calc-total');
    const suvSurchargeRow = document.getElementById('suv-surcharge-row');
    const modalCloseBtn = document.getElementById('modal-close-btn');
    const modalCancelBtn = document.getElementById('modal-cancel-btn');
    const confirmBookingBtn = document.getElementById('confirm-booking-btn');

    // DOM Elements - Add Car Form
    const addCarForm = document.getElementById('add-car-form');

    // Model Specification Map for Luxury Polish
    const modelSpecs = {
        'swift': { seats: '5 Seats', transmission: 'Manual', fuel: 'Petrol • 22 km/l' },
        'hondacity': { seats: '5 Seats', transmission: 'Automatic', fuel: 'Petrol • 18 km/l' },
        'creta': { seats: '5 Seats', transmission: 'Automatic', fuel: 'Diesel • 17 km/l' },
        'fortuner': { seats: '7 Seats', transmission: '4x4 Automatic', fuel: 'Diesel • 12 km/l' },
        'scorpio': { seats: '7 Seats', transmission: 'Manual 4WD', fuel: 'Diesel • 14 km/l' }
    };

    function getSpecs(name, type) {
        const key = (name || '').toLowerCase().replace(/[\s-_]/g, '');
        if (modelSpecs[key]) return modelSpecs[key];
        return type === 'SUV'
            ? { seats: '7 Seats', transmission: 'Automatic', fuel: 'Diesel' }
            : { seats: '5 Seats', transmission: 'Automatic', fuel: 'Petrol' };
    }

    // ==========================================================================
    // Toast Notification System
    // ==========================================================================
    function showToast(message, type = 'success') {
        const container = document.getElementById('toast-container');
        const toast = document.createElement('div');
        toast.className = `toast toast-${type}`;
        
        const iconSvg = type === 'success'
            ? `<svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="#10b981" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><polyline points="20 6 9 17 4 12"></polyline></svg>`
            : `<svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="#f43f5e" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"></circle><line x1="12" y1="8" x2="12" y2="12"></line><line x1="12" y1="16" x2="12.01" y2="16"></line></svg>`;

        toast.innerHTML = `
            <span>${iconSvg}</span>
            <div style="flex:1;">
                <strong style="display:block; font-size:0.75rem; text-transform:uppercase; letter-spacing:0.5px; color:${type === 'success' ? '#10b981' : '#f43f5e'};">
                    ${type === 'success' ? 'Confirmed' : 'Action Required'}
                </strong>
                <span style="font-size:0.85rem; color:#f1f5f9;">${escapeHtml(message)}</span>
            </div>
        `;
        
        container.appendChild(toast);

        setTimeout(() => {
            toast.style.opacity = '0';
            toast.style.transform = 'translateY(10px)';
            toast.style.transition = 'all 0.25s ease';
            setTimeout(() => toast.remove(), 250);
        }, 4000);
    }

    // ==========================================================================
    // Native API Calls (C++ Crow Backend)
    // ==========================================================================
    async function fetchFleet() {
        try {
            const res = await fetch('/api/cars');
            if (!res.ok) throw new Error(`HTTP error ${res.status}`);
            fleet = await res.json();
            renderFleet();
            updateStats();
        } catch (err) {
            console.error('Failed to load fleet:', err);
            fleetContainer.innerHTML = `
                <div class="loading-state">
                    <p style="color:#f43f5e;">Unable to connect to C++ backend at /api/cars</p>
                    <button class="btn btn-secondary" onclick="fetchFleet()">Retry Connection</button>
                </div>
            `;
            showToast('Could not reach C++ Crow server.', 'error');
        }
    }

    async function fetchRentals() {
        try {
            const res = await fetch('/api/rentals');
            if (!res.ok) throw new Error(`HTTP error ${res.status}`);
            rentals = await res.json();
            renderRentals();
            renderHistory();
            updateStats();
        } catch (err) {
            console.error('Failed to load rentals:', err);
            showToast('Failed to load rental transactions.', 'error');
        }
    }

    // ==========================================================================
    // Render Functions
    // ==========================================================================
    function renderFleet() {
        let filtered = fleet;

        // Apply Tab Filter
        if (activeFilter === 'Sedan') {
            filtered = filtered.filter(c => c.type === 'Sedan');
        } else if (activeFilter === 'SUV') {
            filtered = filtered.filter(c => c.type === 'SUV');
        } else if (activeFilter === 'available') {
            filtered = filtered.filter(c => c.available);
        }

        // Apply Search Query
        if (searchQuery) {
            const q = searchQuery.toLowerCase();
            filtered = filtered.filter(c => 
                c.name.toLowerCase().includes(q) || 
                String(c.id).includes(q) ||
                c.type.toLowerCase().includes(q)
            );
        }

        if (filtered.length === 0) {
            fleetContainer.innerHTML = `
                <div class="loading-state">
                    <p>No vehicles match your selected filter or search criteria.</p>
                </div>
            `;
            return;
        }

        fleetContainer.innerHTML = filtered.map(car => {
            const isSUV = car.type === 'SUV';
            const badgeClass = isSUV ? 'badge-suv' : 'badge-sedan';
            const availClass = car.available ? 'avail-free' : 'avail-busy';
            const availText = car.available ? 'Ready for Booking' : 'Currently Reserved';
            const specs = getSpecs(car.name, car.type);

            return `
                <article class="vehicle-card ${car.available ? '' : 'is-rented'}">
                    <div>
                        <div class="card-hero-block">
                            <div class="card-top-row">
                                <span class="vehicle-id-tag font-mono">ID #${car.id}</span>
                                <span class="tier-badge ${badgeClass}">${car.type}</span>
                            </div>
                            <h3 class="vehicle-title">${escapeHtml(car.name)}</h3>
                            <p class="vehicle-category-sub">
                                ${isSUV ? 'Executive All-Terrain SUV (+₹300 Tier Rate)' : 'Premium Luxury Sedan'}
                            </p>
                        </div>

                        <div class="specs-row">
                            <span class="spec-pill">
                                <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M19 21v-2a4 4 0 0 0-4-4H9a4 4 0 0 0-4 4v2"></path><circle cx="12" cy="7" r="4"></circle></svg>
                                ${specs.seats}
                            </span>
                            <span class="spec-pill">
                                <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="3"></circle><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 1 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 1 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 1 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 1 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z"></path></svg>
                                ${specs.transmission}
                            </span>
                            <span class="spec-pill">
                                <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="12" y1="2" x2="12" y2="6"></line><line x1="12" y1="18" x2="12" y2="22"></line><line x1="4.93" y1="4.93" x2="7.76" y2="7.76"></line><line x1="16.24" y1="16.24" x2="19.07" y2="19.07"></line><line x1="2" y1="12" x2="6" y2="12"></line><line x1="18" y1="12" x2="22" y2="12"></line><line x1="4.93" y1="19.07" x2="7.76" y2="16.24"></line><line x1="16.24" y1="7.76" x2="19.07" y2="4.93"></line></svg>
                                ${specs.fuel}
                            </span>
                        </div>
                    </div>

                    <div class="card-bottom">
                        <div class="rate-line">
                            <div>
                                <span class="rate-figure">₹${car.rate.toLocaleString('en-IN')}</span>
                                <span class="rate-period">/ day</span>
                            </div>
                            <span class="availability-pill ${availClass}">
                                <span class="pulse-indicator" style="width:6px; height:6px; background-color:${car.available ? '#10b981' : '#f43f5e'};"></span>
                                ${availText}
                            </span>
                        </div>

                        <div>
                            <button class="btn btn-primary btn-block ${car.available ? '' : 'btn-disabled'}"
                                    ${car.available ? `onclick="openBookingModal(${car.id})"` : 'disabled'}
                                    aria-label="Book ${car.name}">
                                <span>${car.available ? 'Book This Vehicle' : 'Currently Reserved'}</span>
                            </button>
                        </div>
                    </div>
                </article>
            `;
        }).join('');
    }

    function renderRentals() {
        const activeRentals = rentals.filter(r => r.active);
        if (activeRentals.length === 0) {
            rentalsTbody.innerHTML = `
                <tr>
                    <td colspan="7" class="table-empty-state">
                        All fleet vehicles are currently returned and available at the hub.
                    </td>
                </tr>
            `;
            return;
        }

        rentalsTbody.innerHTML = activeRentals.map(r => `
            <tr>
                <td class="font-mono text-cyan">#${r.rentalId}</td>
                <td><strong style="color:#ffffff;">${escapeHtml(r.customer)}</strong></td>
                <td>${escapeHtml(r.carName)} <span class="vehicle-id-tag font-mono">#${r.carId}</span></td>
                <td>${r.days} ${r.days === 1 ? 'day' : 'days'}</td>
                <td class="font-mono text-white font-weight-bold">₹${r.amount.toLocaleString('en-IN', { minimumFractionDigits: 2 })}</td>
                <td>
                    <span class="availability-pill avail-busy" style="font-size:0.7rem; padding:0.2rem 0.55rem;">
                        ● On Road
                    </span>
                </td>
                <td>
                    <button class="btn btn-orange" style="padding:0.35rem 0.85rem; font-size:0.75rem;" onclick="returnCar(${r.rentalId})">
                        Return Vehicle
                    </button>
                </td>
            </tr>
        `).join('');
    }

    function renderHistory() {
        if (rentals.length === 0) {
            historyTbody.innerHTML = `
                <tr>
                    <td colspan="6" class="table-empty-state">No transaction records logged in data/rentals.txt yet.</td>
                </tr>
            `;
            return;
        }

        historyTbody.innerHTML = rentals.map(r => `
            <tr>
                <td class="font-mono">#${r.rentalId}</td>
                <td><strong>${escapeHtml(r.carName)}</strong> <span class="vehicle-id-tag font-mono">#${r.carId}</span></td>
                <td>${escapeHtml(r.customer)}</td>
                <td>${r.days} days</td>
                <td class="font-mono">₹${r.amount.toLocaleString('en-IN', { minimumFractionDigits: 2 })}</td>
                <td>
                    <span class="availability-pill ${r.active ? 'avail-busy' : 'avail-free'}" style="font-size:0.7rem; padding:0.2rem 0.55rem;">
                        ${r.active ? '● Active' : '✓ Returned'}
                    </span>
                </td>
            </tr>
        `).join('');
    }

    function updateStats() {
        const total = fleet.length;
        const available = fleet.filter(c => c.available).length;
        const active = rentals.filter(r => r.active).length;
        const revenue = rentals.reduce((sum, r) => sum + r.amount, 0);

        statTotal.textContent = total;
        statAvailable.textContent = available;
        statRented.textContent = active;
        statRevenue.textContent = '₹' + revenue.toLocaleString('en-IN');
    }

    // ==========================================================================
    // Booking Modal Flow & Live Cost Estimation
    // ==========================================================================
    window.openBookingModal = function(carId) {
        const car = fleet.find(c => c.id === carId);
        if (!car || !car.available) {
            showToast('Selected vehicle is currently unavailable.', 'error');
            return;
        }

        selectedCarForBooking = car;
        bookCarIdInput.value = car.id;
        modalCarName.textContent = car.name;
        modalCarBadge.textContent = car.type;
        modalCarBadge.className = `vehicle-badge ${car.type === 'SUV' ? 'badge-suv' : 'badge-sedan'}`;
        modalCarRate.textContent = `₹${car.rate.toLocaleString('en-IN')}`;
        
        bookDaysInput.value = 1;
        bookCustomerInput.value = '';

        updateLiveBillCalculation();
        bookingModal.classList.add('active');
        bookingModal.setAttribute('aria-hidden', 'false');
        bookCustomerInput.focus();
    };

    function closeBookingModal() {
        bookingModal.classList.remove('active');
        bookingModal.setAttribute('aria-hidden', 'true');
        selectedCarForBooking = null;
    }

    function updateLiveBillCalculation() {
        if (!selectedCarForBooking) return;

        const days = Math.max(1, parseInt(bookDaysInput.value) || 1);
        const rate = selectedCarForBooking.rate;
        const base = days * rate;
        const isSUV = selectedCarForBooking.type === 'SUV';
        const surcharge = isSUV ? 300.0 : 0.0;
        const total = base + surcharge;

        calcDays.textContent = days;
        calcRate.textContent = `₹${rate.toLocaleString('en-IN')}`;
        calcBase.textContent = `₹${base.toLocaleString('en-IN', { minimumFractionDigits: 2 })}`;
        suvSurchargeRow.style.display = isSUV ? 'flex' : 'none';
        calcTotal.textContent = `₹${total.toLocaleString('en-IN', { minimumFractionDigits: 2 })}`;
    }

    bookDaysInput.addEventListener('input', updateLiveBillCalculation);

    modalCloseBtn.addEventListener('click', closeBookingModal);
    modalCancelBtn.addEventListener('click', closeBookingModal);

    // Close on backdrop click
    bookingModal.addEventListener('click', (e) => {
        if (e.target === bookingModal) closeBookingModal();
    });

    // Close on Escape key
    document.addEventListener('keydown', (e) => {
        if (e.key === 'Escape' && bookingModal.classList.contains('active')) {
            closeBookingModal();
        }
    });

    // Booking Submission
    bookForm.addEventListener('submit', async (e) => {
        e.preventDefault();
        if (!selectedCarForBooking) return;

        const carId = parseInt(bookCarIdInput.value);
        const customer = bookCustomerInput.value.trim();
        const days = parseInt(bookDaysInput.value);

        if (!customer) {
            showToast('Please enter the customer name.', 'error');
            return;
        }

        confirmBookingBtn.disabled = true;
        confirmBookingBtn.innerHTML = `<span>Processing C++ Reservation...</span>`;

        try {
            const res = await fetch('/api/book', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ carId, customer, days })
            });

            const data = await res.json();
            if (!res.ok || !data.success) {
                throw new Error(data.message || 'Booking was rejected by C++ engine');
            }

            showToast(`Rental #${data.rentalId} confirmed! ${data.carName} reserved for ${data.customer}. Total: ₹${data.amount.toFixed(2)}`, 'success');
            closeBookingModal();
            await fetchFleet();
            await fetchRentals();
        } catch (err) {
            showToast(err.message, 'error');
        } finally {
            confirmBookingBtn.disabled = false;
            confirmBookingBtn.innerHTML = `<span>Confirm Reservation</span>`;
        }
    });

    // ==========================================================================
    // Return Vehicle Workflow
    // ==========================================================================
    window.returnCar = async function(rentalId) {
        if (!confirm(`Are you sure you want to process the return for Rental #${rentalId}? Vehicle will be restored to available inventory.`)) {
            return;
        }

        try {
            const res = await fetch('/api/return', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ rentalId })
            });

            const data = await res.json();
            if (!res.ok || !data.success) {
                throw new Error(data.message || 'Return operation failed');
            }

            showToast(`Vehicle #${data.carId} (${data.carName}) successfully returned and restored to fleet!`, 'success');
            await fetchFleet();
            await fetchRentals();
        } catch (err) {
            showToast(err.message, 'error');
        }
    };

    // ==========================================================================
    // Add Vehicle Workflow
    // ==========================================================================
    addCarForm.addEventListener('submit', async (e) => {
        e.preventDefault();
        const id = parseInt(document.getElementById('car-id').value);
        const name = document.getElementById('car-name').value.trim();
        const type = document.getElementById('car-type').value;
        const rate = parseFloat(document.getElementById('car-rate').value);

        if (!id || !name || !rate) {
            showToast('Please fill out all required vehicle specifications.', 'error');
            return;
        }

        try {
            const res = await fetch('/api/cars', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ id, name, type, rate })
            });

            const data = await res.json();
            if (!res.ok || !data.success) {
                throw new Error(data.message || 'Could not register vehicle');
            }

            showToast(`Vehicle #${id} (${name} ${type}) successfully added to C++ fleet!`, 'success');
            addCarForm.reset();
            await fetchFleet();
        } catch (err) {
            showToast(err.message, 'error');
        }
    });

    // ==========================================================================
    // Filter & Search Controls
    // ==========================================================================
    filterButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            filterButtons.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            activeFilter = btn.dataset.filter;
            renderFleet();
        });
    });

    fleetSearchInput.addEventListener('input', (e) => {
        searchQuery = e.target.value.trim();
        renderFleet();
    });

    refreshRentalsBtn.addEventListener('click', () => {
        fetchRentals();
        fetchFleet();
        showToast('Fleet and ledger synchronized with C++ database.', 'success');
    });

    function escapeHtml(str) {
        if (!str) return '';
        return String(str)
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/"/g, '&quot;')
            .replace(/'/g, '&#039;');
    }

    // Initial Load
    fetchFleet();
    fetchRentals();
});
